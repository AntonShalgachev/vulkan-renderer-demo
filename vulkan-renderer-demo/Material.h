#pragma once

#include <memory>

namespace vkr
{
    class Shader;
    class Texture;
    class Sampler;

    class Material
    {
    public:
        void setShader(std::shared_ptr<Shader> const& shader) { m_shader = shader; }
        void setTexture(std::shared_ptr<Texture> const& texture) { m_texture = texture; }
        void setSampler(std::shared_ptr<Sampler> const& sampler) { m_sampler = sampler; }

        std::shared_ptr<Shader> const& getShader() const { return m_shader; }
        std::shared_ptr<Texture> const& getTexture() const { return m_texture; }
        std::shared_ptr<Sampler> const& getSampler() const { return m_sampler; }

    private:
        std::shared_ptr<Shader> m_shader;
        std::shared_ptr<Texture> m_texture;
        std::shared_ptr<Sampler> m_sampler;
    };
}
