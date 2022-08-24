#include "Shader.h"

#include "wrapper/ShaderModule.h"
#include <iterator>

vkr::Shader::Shader(vko::Device const& device, Shader::Key const& key) : m_device(device)
{
    std::vector<vko::ShaderModule::Key> const& moduleKeys = key.getModuleKeys();
    if (moduleKeys.empty())
        throw std::runtime_error("Shader key is empty!");

    m_shaderModules.reserve(moduleKeys.size());
    for (vko::ShaderModule::Key const& moduleKey : moduleKeys)
		m_shaderModules.emplace_back(m_device, moduleKey);
}
