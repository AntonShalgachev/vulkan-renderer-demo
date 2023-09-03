#include "ShaderModule.h"

#include "vko/Assert.h"
#include "vko/Device.h"
#include "vko/ShaderModuleProperties.h"

#include "nstl/vector.h"
#include "nstl/span.h"

namespace
{
    VkShaderStageFlagBits getStageFlags(vko::ShaderModuleType type)
    {
        switch (type)
        {
        case vko::ShaderModuleType::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case vko::ShaderModuleType::Geometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case vko::ShaderModuleType::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        assert(false);
        return VK_SHADER_STAGE_VERTEX_BIT;
    }
}

vko::ShaderModule::ShaderModule(Device const& device, nstl::blob_view bytes, ShaderModuleType type, nstl::string entryPoint)
    : m_device(device.getHandle())
    , m_type(type)
    , m_entryPoint(nstl::move(entryPoint))
{
    assert(bytes.size() % 4 == 0);
    static_assert(sizeof(uint32_t) == 4 * sizeof(unsigned char));

    nstl::vector<uint32_t> code;
    code.resize(bytes.size() / 4);
    memcpy(code.data(), bytes.data(), bytes.size());

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = bytes.size();
    createInfo.pCode = code.data();

    VKO_VERIFY(vkCreateShaderModule(m_device, &createInfo, &m_allocator.getCallbacks(), &m_handle.get()));
}

vko::ShaderModule::~ShaderModule()
{
    if (m_handle)
        vkDestroyShaderModule(m_device, m_handle, &m_allocator.getCallbacks());

    m_handle = nullptr;
}

VkPipelineShaderStageCreateInfo vko::ShaderModule::createStageCreateInfo() const
{
    VkPipelineShaderStageCreateInfo stageCreateInfo{};
    stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageCreateInfo.stage = getStageFlags(m_type);
    stageCreateInfo.module = m_handle;
    stageCreateInfo.pName = m_entryPoint.c_str();

    return stageCreateInfo;
}
