#include "renderer.h"

#include "context.h"
#include "conversions.h"

#include "renderpass.h"
#include "framebuffer.h"
#include "renderstate.h"
#include "descriptorgroup.h"
#include "buffer.h"

#include "vko/Semaphore.h"
#include "vko/Fence.h"
#include "vko/CommandPool.h"
#include "vko/CommandBuffers.h"
#include "vko/Queue.h"
#include "vko/Device.h"
#include "vko/Instance.h"
#include "vko/PhysicalDevice.h"
#include "vko/PhysicalDeviceSurfaceParameters.h"
#include "vko/Window.h"

#include "common/fmt.h"

#include "nstl/array.h"

namespace
{
    VkFormat find_supported_format(vko::PhysicalDevice const& physicalDevice, nstl::span<VkFormat const> candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice.getHandle(), format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                return format;
            if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                return format;
        }

        assert(false);
        return VkFormat{};
    }

    VkFormat find_depth_format(vko::PhysicalDevice const& physicalDevice)
    {
        VkFormat candidates[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
        return find_supported_format(physicalDevice, candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    VkSurfaceFormatKHR find_surface_format(nstl::span<VkSurfaceFormatKHR const> surface_formats)
    {
        if (surface_formats.empty())
            return { VK_FORMAT_UNDEFINED , VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

        for (const auto& surface_format : surface_formats)
            if (surface_format.format == VK_FORMAT_B8G8R8A8_SRGB && surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return surface_format;

        return surface_formats[0];
    }
}

struct gfx_vk::renderer::frame_resources
{
    vko::Semaphore image_available_semaphore;
    vko::Semaphore render_finished_semaphore;
    vko::Fence in_flight_fence;

    vko::CommandPool command_pool;
    vko::CommandBuffers command_buffers;
};

gfx_vk::renderer::renderer(context& context, vko::Window& window, renderer_config const& config) : m_context(context)
{
    create_swapchain(window);
    create_frame_resources(config);
}

gfx_vk::renderer::~renderer() = default;

gfx::framebuffer_handle gfx_vk::renderer::acquire_main_framebuffer()
{
    // TODO vkAcquireNextImageKHR

    assert(m_in_frame);

    m_swapchain_image_index = (m_swapchain_image_index + 1) % m_fake_color_images.size();

    return m_fake_framebuffers[m_swapchain_image_index];
}

float gfx_vk::renderer::get_main_framebuffer_aspect() const
{
    if (!m_swapchain)
        return 1.0f * 1900 / 1000; // WTF

    VkExtent2D extent = m_swapchain->get_extent();
    return 1.0f * extent.width / extent.height;
}

void gfx_vk::renderer::wait_for_next_frame()
{
    m_frame_resources[m_next_resources_index].in_flight_fence.wait();
}

void gfx_vk::renderer::begin_frame()
{
    m_resources_index = m_next_resources_index;
    m_next_resources_index = (m_next_resources_index + 1) % m_frame_resources.size();

    frame_resources& resources = m_frame_resources[m_resources_index];

    resources.in_flight_fence.wait();
    resources.in_flight_fence.reset();

    resources.command_pool.reset();
    resources.command_buffers.begin(0);

    m_in_frame = true;
}

void gfx_vk::renderer::renderpass_begin(gfx::renderpass_begin_params const& params)
{
    frame_resources& resources = m_frame_resources[m_resources_index];

    framebuffer& fb = m_context.get_resources().get_framebuffer(params.framebuffer);

    nstl::vector<VkClearValue> clear_values;
    for (gfx::image_type type : fb.get_attachment_types())
    {
        switch (type)
        {
        case gfx::image_type::color:
            clear_values.push_back({ .color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f} } });
            break;
        case gfx::image_type::depth:
            clear_values.push_back({ .depthStencil = { 1.0f, 0 } });
            break;
        }
    }

    VkRect2D render_area = {
        .offset = {0, 0},
        .extent = fb.get_extent(),
    };

    VkRenderPassBeginInfo info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_context.get_resources().get_renderpass(params.renderpass).get_handle(),
        .framebuffer = fb.get_handle(),
        .renderArea = render_area,
        .clearValueCount = static_cast<uint32_t>(clear_values.size()),
        .pClearValues = clear_values.data(),
    };

    vkCmdBeginRenderPass(resources.command_buffers.getHandle(0), &info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = 1.0f * fb.get_extent().width,
        .height = 1.0f * fb.get_extent().height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(resources.command_buffers.getHandle(0), 0, 1, &viewport);

    m_current_renderpass = params.renderpass;
    m_current_framebuffer = params.framebuffer;
}

void gfx_vk::renderer::renderpass_end()
{
    m_current_renderpass = nullptr;
    m_current_framebuffer = nullptr;

    frame_resources& resources = m_frame_resources[m_resources_index];

    vkCmdEndRenderPass(resources.command_buffers.getHandle(0));
}

void gfx_vk::renderer::draw_indexed(gfx::draw_indexed_args const& args)
{
    assert(m_current_renderpass != nullptr);

    frame_resources& resources = m_frame_resources[m_resources_index];

    VkCommandBuffer command_buffer = resources.command_buffers.getHandle(0);

    renderstate& rs = m_context.get_resources().get_renderstate(args.renderstate);

    // TODO track what's already bound

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, rs.get_handle());

    nstl::vector<VkDescriptorSet> sets;
    for (gfx::descriptorgroup_handle handle : args.descriptorgroups)
        sets.push_back(m_context.get_resources().get_descriptorgroup(handle).get_handle());
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, rs.get_params().layout, 0, sets.size(), sets.data(), 0, nullptr);

    nstl::vector<VkBuffer> vertex_buffers;
    nstl::vector<VkDeviceSize> vertex_buffers_offset;
    for (gfx::buffer_handle handle : args.vertex_buffers)
    {
        vertex_buffers.push_back(m_context.get_resources().get_buffer(handle).get_handle());
        vertex_buffers_offset.push_back(0);
    }
    vkCmdBindVertexBuffers(command_buffer, 0, args.vertex_buffers.size(), vertex_buffers.data(), vertex_buffers_offset.data());

    vkCmdBindIndexBuffer(command_buffer, m_context.get_resources().get_buffer(args.index_buffer).get_handle(), 0, utils::get_index_type(args.index_type));

    VkRect2D scissor{};
    if (args.scissor)
    {
        assert(args.scissor->size.x > 0);
        assert(args.scissor->size.y > 0);

        scissor = {
            .offset = { args.scissor->offset.x, args.scissor->offset.y },
            .extent = { static_cast<uint32_t>(args.scissor->size.x), static_cast<uint32_t>(args.scissor->size.y) },
        };
    }
    else
    {
        framebuffer& fb = m_context.get_resources().get_framebuffer(m_current_framebuffer);

        scissor = {
            .offset = { 0, 0 },
            .extent = fb.get_extent(),
        };
    }
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    vkCmdDrawIndexed(command_buffer, args.index_count, 1, args.first_index, args.vertex_offset, 0);
}

void gfx_vk::renderer::submit()
{
    m_in_frame = false;

    frame_resources& resources = m_frame_resources[m_resources_index];

    resources.command_buffers.end(0);
    resources.command_buffers.submit(0, m_context.get_device().getGraphicsQueue(), nullptr /*&resources.render_finished_semaphore*/, nullptr /*&resources.image_available_semaphore*/, &resources.in_flight_fence); // TODO use the semaphore
}

//////////////////////////////////////////////////////////////////////////

void gfx_vk::renderer::create_swapchain(vko::Window& window)
{
    // TODO use gfx::image_format
    VkSurfaceFormatKHR vk_surface_format = find_surface_format(m_context.get_physical_device_surface_parameters().formats);
    VkFormat vk_depth_format = find_depth_format(m_context.get_physical_device());

    vko::Device const& device = m_context.get_device();

    // TODO remove
    gfx::image_format surface_format = gfx::image_format::b8g8r8a8_srgb;
    gfx::image_format depth_format = gfx::image_format::d32_float;
    assert(utils::get_format(surface_format) == vk_surface_format.format);
    assert(utils::get_format(depth_format) == vk_depth_format);

    // TODO use create_renderpass?
    m_renderpass = m_context.get_resources().create_renderpass({
        .color_attachment_formats = nstl::array{ surface_format },
        .depth_stencil_attachment_format = depth_format,

        .has_presentable_images = true,
        .keep_depth_values_after_renderpass = false,
    });

    m_context.get_instance().setDebugName(device.getHandle(), m_context.get_resources().get_renderpass(m_renderpass).get_handle(), "Main renderpass");

    // disabled while the old renderer creates its own swapchain
//     m_swapchain = nstl::make_unique<swapchain>(*m_context, window, m_renderpass->get_handle(), surface_format, depth_format);

    size_t swapchain_images_count = 4; // TODO: should be retrieved from the swapchain

    for (size_t i = 0; i < swapchain_images_count; i++)
    {
        m_fake_color_images.push_back(m_context.get_resources().create_image({
            .width = window.getFramebufferWidth(),
            .height = window.getFramebufferHeight(),
            .format = surface_format,
            .type = gfx::image_type::color,
            .usage = gfx::image_usage::color,
        }));
    }

    m_fake_depth_image = m_context.get_resources().create_image({
        .width = window.getFramebufferWidth(),
        .height = window.getFramebufferHeight(),
        .format = depth_format,
        .type = gfx::image_type::depth,
        .usage = gfx::image_usage::depth,
    });

    for (size_t i = 0; i < swapchain_images_count; i++)
    {
        m_fake_framebuffers.push_back(m_context.get_resources().create_framebuffer({
            .attachments = nstl::array{ m_fake_color_images[i], m_fake_depth_image },
            .renderpass = m_renderpass,
        }));
    }

    m_swapchain_image_index = swapchain_images_count - 1;
}

void gfx_vk::renderer::create_frame_resources(renderer_config const& config)
{
    VkDevice device = m_context.get_device().getHandle();
    vko::Instance const& instance = m_context.get_instance();

    for (auto i = 0; i < config.max_frames_in_flight; i++)
    {
        vko::CommandPool command_pool{ device, m_context.get_device().getGraphicsQueue().getFamily() };
        vko::CommandBuffers command_buffers{ command_pool.allocate(1) };

        m_frame_resources.push_back({
            .image_available_semaphore{ device },
            .render_finished_semaphore{ device },
            .in_flight_fence{ device },
            .command_pool = nstl::move(command_pool),
            .command_buffers = nstl::move(command_buffers),
        });

        frame_resources& resources = m_frame_resources.back();

        instance.setDebugName(device, resources.image_available_semaphore.getHandle(), common::format("Image available semaphore (frame {})", i));
        instance.setDebugName(device, resources.render_finished_semaphore.getHandle(), common::format("Render finished semaphore (frame {})", i));
        instance.setDebugName(device, resources.in_flight_fence.getHandle(), common::format("In-flight fence (frame {})", i));
        instance.setDebugName(device, resources.command_pool.getHandle(), common::format("Main command pool (frame {})", i));

        for (size_t index = 0; index < resources.command_buffers.getSize(); index++)
            instance.setDebugName(device, resources.command_buffers.getHandle(index), common::format("Command buffer {} (frame {})", index, i));
    }
}
