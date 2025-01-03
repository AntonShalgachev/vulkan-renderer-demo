import colorama
colorama.init(convert=True)

import argparse
import json
import itertools
import logging
import os
import shutil
import subprocess
import time
from typing import Dict, Iterable, List, Tuple
import yaml


GLSL_COMPILER = 'glslc'
GLSL_OPTIONS = '-g -O0'


YAML_EXTENSIONS = ('.yml', '.yaml')


class CustomFormatter(logging.Formatter):
    grey = "\x1b[38;20m"
    yellow = "\x1b[33;20m"
    red = "\x1b[31;20m"
    bold_red = "\x1b[31;1m"
    reset = "\x1b[0m"
    format = "[%(levelname)s] %(message)s (%(filename)s:%(lineno)d)"

    FORMATS = {
        logging.DEBUG: grey + format + reset,
        logging.INFO: grey + format + reset,
        logging.WARNING: yellow + format + reset,
        logging.ERROR: red + format + reset,
        logging.CRITICAL: bold_red + format + reset
    }

    def format(self, record):
        log_fmt = self.FORMATS.get(record.levelno)
        formatter = logging.Formatter(log_fmt)
        return formatter.format(record)


logger = logging.getLogger(__name__)
handler = logging.StreamHandler()
handler.setFormatter(CustomFormatter())
logger.addHandler(handler)

class ShaderMetadata:
    configuration: Dict[str, str]
    cmdline: str
    path: str
    compiler_version: List[str]


def execute_command(command: str, cwd: str = None):
    logger.debug(command)

    process = subprocess.Popen(command, cwd=cwd, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE, universal_newlines=True)
    stdout, stderr = process.communicate()

    if process.returncode != 0:
        for line in stdout.splitlines():
            logger.critical(line)
        for line in stderr.splitlines():
            logger.critical(line)

        raise RuntimeError(
            'Command finished with error code {}'.format(process.returncode))

    return stdout


def compile(compiler:str, input: str, output: str, name: str, option_names: List[str], option_values: List[str]):
    configuration = {name: value for name, value in zip(option_names, option_values) if value is not None}
    option_strings = list('{}{}{}'.format(name, '=' if len(str(value)) > 0 else '', value)
                   for name, value in configuration.items())

    definitions = ' '.join(['-D' + option for option in option_strings])

    configuration_directory = '-'.join(option_strings)
    relative_output_directory = os.path.join('shaders', configuration_directory)
    output_directory = os.path.join(output, relative_output_directory)

    if not os.path.exists(output_directory):
        os.makedirs(output_directory)

    relative_output_file = os.path.join(relative_output_directory, name + '.spv')
    output_file = os.path.join(output, relative_output_file)

    compilation_options = '{} {}'.format(GLSL_OPTIONS, definitions).strip()
    compilation_command = '{executable} {input} -o {output} {options}'.format(
        executable=compiler,
        input=input,
        output=output_file,
        options=compilation_options)
        
    try:
        execute_command(compilation_command)
    except Exception as e:
        logger.critical('Failed to compile shader with [{}]'.format(' '.join(option_strings)))
        raise

    version_output = execute_command('{} --version'.format(compiler))

    metadata = ShaderMetadata()
    metadata.configuration = configuration
    metadata.cmdline = compilation_options
    metadata.path = relative_output_file
    metadata.compiler_version = [v for v in version_output.splitlines() if v]
    
    return metadata


def package_shader(compiler:str, manifest_path: str, output: str):
    if not os.path.exists(manifest_path):
        raise RuntimeError("Shader manifest {} doesn't exist! PWD: '{}'".format(manifest_path, os.getcwd()))

    if not os.path.isfile(manifest_path):
        raise RuntimeError("Shader manifest '{}' is not a file".format(manifest_path))

    shader_path, manifest_ext = os.path.splitext(manifest_path)
    if manifest_ext not in YAML_EXTENSIONS:
        logger.warning("The file '{}' doesn't have a YAML file extension. Are you sure you are passing the path to the shader manifest?".format(manifest_path))

    logger.info("Compiling '{}'".format(manifest_path))

    manifest = None
    with open(manifest_path, 'r') as f:
        try:
            manifest = yaml.safe_load(f)
        except yaml.parser.ParserError as e:
            raise RuntimeError('Failed to load the YAML shader manifest') from e

    name = os.path.basename(shader_path)

    options = manifest['options']

    option_names = tuple(name for name, values in options.items())
    ls = (values for name, values in options.items())
    configurations = itertools.product(*ls)

    if os.path.exists(output):
        shutil.rmtree(output)

    s = time.time()

    metadatas = []

    for configuration in configurations:
        metadata = compile(compiler, shader_path, output, name, option_names, configuration)
        metadatas.append(metadata)

    e = time.time()
    logger.debug('Time elapsed: {}s'.format(e-s))

    package_info = {'options': option_names, 'variants': metadatas}

    package_info_file = os.path.join(output, 'package.json')
    with open(package_info_file, 'w') as f:
        json.dump(package_info, f, indent=4, default=lambda x: x.__dict__)


def find_shaders(input: str, output: str) -> Iterable[Tuple[str, str]]:
    if os.path.isfile(input):
        return [(input, output)]

    shaders: List[Tuple[str, str]] = []

    for root, _, files in os.walk(input):
        for file in files:
            filename_root, ext = os.path.splitext(file)
            if ext not in YAML_EXTENSIONS:
                continue
            
            manifest_path = os.path.join(root, file)

            relative_path = os.path.relpath(root, input)
            shader_name = filename_root
            output_path = os.path.join(output, relative_path, shader_name)

            shaders.append((manifest_path, output_path))

    return shaders


def main():
    parser = argparse.ArgumentParser(description='Package shader(s)')
    parser.add_argument('input', metavar='IN', type=str,
                        help='Path to the directory with manifest.yml in it')
    parser.add_argument('--output', '-o', metavar='OUT', required=True,
                        type=str, help='Output directory of the packaged shader')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose logs')

    args = parser.parse_args()

    logger.setLevel(logging.DEBUG if args.verbose else logging.INFO)

    search_path = os.environ['PATH']
    if 'VULKAN_SDK' in os.environ:
        vulkan_bin_path = os.path.join(os.environ['VULKAN_SDK'], 'Bin')
        search_path = vulkan_bin_path + ';' + search_path

    compiler_path = shutil.which(GLSL_COMPILER, path=search_path)
    if compiler_path is None:
        raise RuntimeError(
            "GLSL compiler '{}' hasn't been found. Make sure it is in your PATH or in VULKAN_SDK/Bin".format(GLSL_COMPILER))

    if not os.path.exists(args.input):
        raise RuntimeError("The path '{}' doesn't exist".format(args.input))

    for input, output in find_shaders(args.input, args.output):
        package_shader(compiler_path, input, output)


if __name__ == '__main__':
    main()
