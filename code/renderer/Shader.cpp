#include "Shader.h"

#include "wrapper/ShaderModule.h"
#include <iterator>
#include <vector>
#include <fstream>

namespace
{
    std::vector<unsigned char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
            throw std::runtime_error("failed to open file!");

        std::streamsize fileSize = file.tellg();
        std::vector<unsigned char> buffer(static_cast<std::size_t>(fileSize));

        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize); // TODO check if okay
        file.close();

        return buffer;
    }
}

vkr::Shader::Shader(vko::Device const& device, Shader::Key const& key) : m_device(device)
{
    std::vector<ShaderModuleKey> const& moduleKeys = key.getModuleKeys();
    if (moduleKeys.empty())
        throw std::runtime_error("Shader key is empty!");

    m_shaderModules.reserve(moduleKeys.size());
    for (ShaderModuleKey const& moduleKey : moduleKeys)
    {
        auto bytes = readFile(moduleKey.path);
        m_shaderModules.emplace_back(m_device, bytes, moduleKey.type, moduleKey.entryPoint);
    }
}

std::size_t std::hash<vkr::Shader::Key>::operator()(vkr::Shader::Key const& rhs) const
{
    return rhs.computeHash();
}

std::size_t std::hash<vkr::ShaderModuleKey>::operator()(vkr::ShaderModuleKey const& rhs) const
{
    return rhs.computeHash();
}
