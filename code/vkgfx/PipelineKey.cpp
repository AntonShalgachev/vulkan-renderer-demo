#include "PipelineKey.h"

size_t vkgfx::UniformConfiguration::hash() const
{
    return nstl::hash_values(hasBuffer, hasAlbedoTexture, hasNormalMap);
}

size_t vkgfx::VertexConfiguration::Binding::hash() const
{
    return nstl::hash_values(stride);
}

size_t vkgfx::VertexConfiguration::Attribute::hash() const
{
    return nstl::hash_values(binding, location, offset);
}

size_t vkgfx::VertexConfiguration::hash() const
{
    return nstl::hash_values(bindings, attributes, topology);
}

size_t vkgfx::RenderConfiguration::hash() const
{
    return nstl::hash_values(cullBackfaces, wireframe, depthTest, alphaBlending);
}

size_t vkgfx::PushConstantRange::hash() const
{
    return nstl::hash_values(offset, size);
}

size_t vkgfx::DescriptorSetLayoutKey::hash() const
{
    return nstl::hash_values(uniformConfig);
}

size_t vkgfx::PipelineLayoutKey::hash() const
{
    return nstl::hash_values(uniformConfigs, pushConstantRanges);
}

size_t vkgfx::PipelineKey::hash() const
{
    return nstl::hash_values(shaderHandles, uniformConfigs, vertexConfig, renderConfig, pushConstantRanges, isShadowmap);
}
