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
from typing import Dict, List
import yaml


GLSL_COMPILER = 'glslc'
GLSL_OPTIONS = '-g -O0'


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
    command = '{executable} {input} -o {output} {options}'.format(
        executable=compiler,
        input=input,
        output=output_file,
        options=compilation_options)
        
    execute_command(command)

    metadata = ShaderMetadata()
    metadata.configuration = configuration
    metadata.cmdline = compilation_options
    metadata.path = relative_output_file
    
    return metadata


def package_shader(compiler:str, input: str, output: str):
    if not os.path.isfile(input):
        raise RuntimeError("Shader {} doesn't exist! PWD: '{}'".format(input, os.getcwd()))

    manifest = None
    manifest_path = input + '.yml'
    with open(manifest_path, 'r') as f:
        manifest = yaml.safe_load(f)

    name = os.path.basename(input)

    options = manifest['options']

    option_names = tuple(name for name, values in options.items())
    ls = (values for name, values in options.items())
    configurations = itertools.product(*ls)

    if os.path.exists(output):
        shutil.rmtree(output)

    s = time.time()

    metadatas = []

    for configuration in configurations:
        metadata = compile(compiler, input, output, name, option_names, configuration)
        metadatas.append(metadata)

    e = time.time()
    logger.info('Time elapsed: {}s'.format(e-s))

    package_info = {'options': option_names, 'variants': metadatas}

    package_info_file = os.path.join(output, 'package.json')
    with open(package_info_file, 'w') as f:
        json.dump(package_info, f, indent=4, default=lambda x: x.__dict__)


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

    package_shader(compiler_path, args.input, args.output)


if __name__ == '__main__':
    main()
