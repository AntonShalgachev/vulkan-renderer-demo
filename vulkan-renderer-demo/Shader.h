#pragma once
#include <string>

#include "ShaderModule.h"
#include "Object.h"

namespace vkr
{
    class Shader : vkr::Object
    {
    public:
    	Shader(Application const& app, std::string const& vertexPath, std::string const& fragmentPath, std::string const& vertexEntryPoint = "main", std::string const& fragmentEntryPoint = "main");

        ShaderModule createVertexModule() const;
        ShaderModule createFragmentModule() const;

    private:
        std::string m_vertexPath;
        std::string m_fragmentPath;
        std::string m_vertexEntryPoint;
        std::string m_fragmentEntryPoint;
    };
}
