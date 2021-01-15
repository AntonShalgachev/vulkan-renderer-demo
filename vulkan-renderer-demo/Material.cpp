#include "Material.h"

void vkr::Material::setTexture(std::shared_ptr<Texture> const& texture)
{
    m_texture = texture;
}

void vkr::Material::setSampler(std::shared_ptr<Sampler> const& sampler)
{
    m_sampler = sampler;
}
