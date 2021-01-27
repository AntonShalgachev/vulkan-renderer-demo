#include "Shader.h"

vkr::Shader::Shader(Application const& app, std::string const& vertexPath, std::string const& fragmentPath, std::string const& vertexEntryPoint, std::string const& fragmentEntryPoint)
    : Object(app)
    , m_vertexPath(vertexPath)
    , m_fragmentPath(fragmentPath)
    , m_vertexEntryPoint(vertexEntryPoint)
    , m_fragmentEntryPoint(fragmentEntryPoint)
{

}

vkr::ShaderModule vkr::Shader::createVertexModule() const
{
    return ShaderModule{getApp(), m_vertexPath, ShaderModule::Type::Vertex, m_vertexEntryPoint};
}

vkr::ShaderModule vkr::Shader::createFragmentModule() const
{
    return ShaderModule{ getApp(), m_fragmentPath, ShaderModule::Type::Fragment, m_fragmentEntryPoint };
}
