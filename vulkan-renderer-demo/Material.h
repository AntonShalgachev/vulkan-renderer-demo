#pragma once

#include <memory>

namespace vkr
{
    class Texture;
    class Sampler;

    class Material
    {
    public:
        void setTexture(std::shared_ptr<Texture> const& texture);
        void setSampler(std::shared_ptr<Sampler> const& sampler);

        Texture const& getTexture() const { return *m_texture; }
        Sampler const& getSampler() const { return *m_sampler; }

    private:
        std::shared_ptr<Texture> m_texture;
        std::shared_ptr<Sampler> m_sampler;
    };
}
