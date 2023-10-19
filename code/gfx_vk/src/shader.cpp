#include "shader.h"

#include "context.h"

#include "fs/file.h"

#include "nstl/vector.h"

namespace
{
    VkShaderStageFlagBits get_stage_flags(gfx::shader_stage stage)
    {
        switch (stage)
        {
        case gfx::shader_stage::vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case gfx::shader_stage::geometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case gfx::shader_stage::fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        assert(false);
        return VK_SHADER_STAGE_VERTEX_BIT;
    }
}

gfx_vk::shader::shader(context& context, gfx::shader_params const& params)
    : m_context(context)
    , m_stage(params.stage)
    , m_entry_point(params.entry_point)
{
    fs::file f{ params.filename, fs::open_mode::read };

    size_t byte_size = f.size();
    assert(byte_size % 4 == 0); // SPIR-V module is a stream of 32-bit words

    nstl::vector<uint32_t> words;
    words.resize(byte_size / 4);
    f.read(words.data(), byte_size);

    VkShaderModuleCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = byte_size,
        .pCode = words.data(),
    };

    GFX_VK_VERIFY(vkCreateShaderModule(m_context.get_device_handle(), &info, &m_context.get_allocator(), &m_handle.get()));
}

gfx_vk::shader::~shader()
{
    if (!m_handle)
        return;

    vkDestroyShaderModule(m_context.get_device_handle(), m_handle, &m_context.get_allocator());
}

VkPipelineShaderStageCreateInfo gfx_vk::shader::create_stage_create_info() const
{
    VkPipelineShaderStageCreateInfo stageCreateInfo{};
    stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageCreateInfo.stage = get_stage_flags(m_stage);
    stageCreateInfo.module = m_handle.get();
    stageCreateInfo.pName = m_entry_point.c_str();

    return stageCreateInfo;
}
