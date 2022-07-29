#pragma once

#include <memory>
#include "glm.h"

// TODO maybe remove?
#include "Shader.h"

namespace vkr
{
    class Texture;
    class Sampler;

    class Material
    {
    public:
        void setShaderKey(Shader::Key shaderKey) { m_shaderKey = std::move(shaderKey); }
        void setTexture(std::shared_ptr<Texture> const& texture) { m_texture = texture; }
        void setNormalMap(std::shared_ptr<Texture> const& normalMap) { m_normalMap = normalMap; }
        void setSampler(std::shared_ptr<Sampler> const& sampler) { m_sampler = sampler; }
        void setColor(glm::vec4 const& color) { m_color = color; }

        Shader::Key const& getShaderKey() const { return m_shaderKey; }
        std::shared_ptr<Texture> const& getTexture() const { return m_texture; }
        std::shared_ptr<Texture> const& getNormalMap() const { return m_normalMap; }
        std::shared_ptr<Sampler> const& getSampler() const { return m_sampler; }
        glm::vec4 const& getColor() const { return m_color; }

    private:
        Shader::Key m_shaderKey;
        std::shared_ptr<Texture> m_texture;
        std::shared_ptr<Texture> m_normalMap;
        std::shared_ptr<Sampler> m_sampler;

        glm::vec4 m_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    };
}
