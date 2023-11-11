#include "renderer.h"

#include "context.h"
#include "conversions.h"

#include "renderpass.h"
#include "framebuffer.h"
#include "renderstate.h"
#include "descriptorgroup.h"
#include "buffer.h"
#include "command_pool.h"

#include "nstl/array.h"
#include "nstl/static_vector.h"

namespace
{
    class semaphore
    {
    public:
        semaphore(gfx_vk::context& context) : m_context(context)
        {
            VkSemaphoreCreateInfo info{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

            GFX_VK_VERIFY(vkCreateSemaphore(m_context.get_device_handle(), &info, &m_context.get_allocator(), &m_handle.get()));
        }
        semaphore(semaphore&&) = default;
        ~semaphore()
        {
            vkDestroySemaphore(m_context.get_device_handle(), m_handle, &m_context.get_allocator());
            m_handle = nullptr;
        }

        VkSemaphore const& get_handle() const { return m_handle; }

    private:
        gfx_vk::context& m_context;
        gfx_vk::unique_handle<VkSemaphore> m_handle;
    };

    class fence
    {
    public:
        fence(gfx_vk::context& context) : m_context(context)
        {
            VkFenceCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT,
            };

            GFX_VK_VERIFY(vkCreateFence(m_context.get_device_handle(), &info, &m_context.get_allocator(), &m_handle.get()));
        }
        fence(fence&&) = default;
        ~fence()
        {
            vkDestroyFence(m_context.get_device_handle(), m_handle, &m_context.get_allocator());
            m_handle = nullptr;
        }

        VkFence const& get_handle() const { return m_handle; }

        void wait() const
        {
            GFX_VK_VERIFY(vkWaitForFences(m_context.get_device_handle(), 1, &m_handle.get(), VK_TRUE, UINT64_MAX));
        }

        void reset() const
        {
            GFX_VK_VERIFY(vkResetFences(m_context.get_device_handle(), 1, &m_handle.get()));
        }

    private:
        gfx_vk::context& m_context;
        gfx_vk::unique_handle<VkFence> m_handle;
    };
}

struct gfx_vk::renderer::frame_resources
{
    semaphore image_available_semaphore;
    semaphore render_finished_semaphore;
    fence in_flight_fence;

    command_pool command_pool;
    VkCommandBuffer command_buffer;
};

gfx_vk::renderer::renderer(context& context, size_t w, size_t h, renderer_config const& config) : m_context(context)
{
    create_swapchain(w, h);
    create_frame_resources(config);
}

gfx_vk::renderer::~renderer() = default;

void gfx_vk::renderer::resize_main_framebuffer(size_t w, size_t h)
{
    m_context.on_surface_changed();
    m_swapchain->resize(w, h);
}

gfx::framebuffer_handle gfx_vk::renderer::acquire_main_framebuffer()
{
    assert(m_in_frame);

    frame_resources& resources = get_current_frame_resources();

    uint32_t image_index = 0;
    [[maybe_unused]] VkResult result = vkAcquireNextImageKHR(m_context.get_device_handle(), m_swapchain->get_handle(), UINT64_MAX, resources.image_available_semaphore.get_handle(), VK_NULL_HANDLE, &image_index);
    assert(result == VK_SUCCESS); // TODO handle swapchain resize

    m_swapchain_image_index = image_index;

    return m_swapchain->get_framebuffers()[image_index];
}

float gfx_vk::renderer::get_main_framebuffer_aspect() const
{
    VkExtent2D extent = m_swapchain->get_extent();
    return 1.0f * extent.width / extent.height;
}

void gfx_vk::renderer::begin_resource_update()
{
    m_context.increment_mutable_resource_index();

    get_current_frame_resources().in_flight_fence.wait();
    get_current_frame_resources().in_flight_fence.reset();
}

void gfx_vk::renderer::begin_frame()
{
    VkCommandPoolResetFlags flags = 0;
    GFX_VK_VERIFY(vkResetCommandPool(m_context.get_device_handle(), get_current_frame_resources().command_pool.get_handle(), flags));

    VkCommandBufferBeginInfo info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    GFX_VK_VERIFY(vkBeginCommandBuffer(get_current_frame_resources().command_buffer, &info));

    m_in_frame = true;
}

void gfx_vk::renderer::renderpass_begin(gfx::renderpass_begin_params const& params)
{
    frame_resources& resources = get_current_frame_resources();

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

    vkCmdBeginRenderPass(resources.command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = 1.0f * fb.get_extent().width,
        .height = 1.0f * fb.get_extent().height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(resources.command_buffer, 0, 1, &viewport);

    m_current_renderpass = params.renderpass;
    m_current_framebuffer = params.framebuffer;
}

void gfx_vk::renderer::renderpass_end()
{
    m_current_renderpass = nullptr;
    m_current_framebuffer = nullptr;

    vkCmdEndRenderPass(get_current_frame_resources().command_buffer);
}

void gfx_vk::renderer::draw_indexed(gfx::draw_indexed_args const& args)
{
    assert(m_current_renderpass != nullptr);

    VkCommandBuffer command_buffer = get_current_frame_resources().command_buffer;

    renderstate& rs = m_context.get_resources().get_renderstate(args.renderstate);

    // TODO track what's already bound

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, rs.get_handle());

    nstl::static_vector<VkDescriptorSet, 5> sets;
    for (gfx::descriptorgroup_handle handle : args.descriptorgroups)
        sets.push_back(m_context.get_resources().get_descriptorgroup(handle).get_current_handle());
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, rs.get_params().layout, 0, static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);

    nstl::static_vector<VkBuffer, 5> vertex_buffers;
    nstl::static_vector<VkDeviceSize, 5> vertex_buffers_offset;
    for (auto&& [buffer, offset] : args.vertex_buffers)
    {
        vertex_buffers.push_back(m_context.get_resources().get_buffer(buffer).get_current_handle());
        vertex_buffers_offset.push_back(offset);
    }
    vkCmdBindVertexBuffers(command_buffer, 0, static_cast<uint32_t>(args.vertex_buffers.size()), vertex_buffers.data(), vertex_buffers_offset.data());

    vkCmdBindIndexBuffer(command_buffer, m_context.get_resources().get_buffer(args.index_buffer.buffer).get_current_handle(), args.index_buffer.offset, utils::get_index_type(args.index_type));

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

    assert(args.vertex_offset <= INT32_MAX);
    vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(args.index_count), static_cast<uint32_t>(args.instance_count), static_cast<uint32_t>(args.first_index), static_cast<int32_t>(args.vertex_offset), 0);
}

void gfx_vk::renderer::submit()
{
    m_in_frame = false;

    frame_resources& resources = get_current_frame_resources();

    GFX_VK_VERIFY(vkEndCommandBuffer(get_current_frame_resources().command_buffer));

    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &resources.image_available_semaphore.get_handle(),
            .pWaitDstStageMask = &wait_stage,
            .commandBufferCount = 1,
            .pCommandBuffers = &resources.command_buffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &resources.render_finished_semaphore.get_handle(),
        };

        GFX_VK_VERIFY(vkQueueSubmit(m_context.get_instance().get_graphics_queue_handle(), 1, &info, resources.in_flight_fence.get_handle()));
    }

    nstl::array wait_semaphores{ resources.render_finished_semaphore.get_handle()};
    nstl::array swapchains = { m_swapchain->get_handle() };

    VkPresentInfoKHR info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size()),
        .pWaitSemaphores = wait_semaphores.data(),
        .swapchainCount = static_cast<uint32_t>(swapchains.size()),
        .pSwapchains = swapchains.data(),
        .pImageIndices = &m_swapchain_image_index,
        .pResults = nullptr,
    };

    [[maybe_unused]] VkResult result = vkQueuePresentKHR(m_context.get_instance().get_present_queue_handle(), &info);
    assert(result == VK_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////

void gfx_vk::renderer::create_swapchain(size_t w, size_t h)
{
    // TODO don't rely on these formats being supported
    gfx_vk::surface_format surface_format = {
        .format = gfx::image_format::b8g8r8a8_srgb,
        .color_space = color_space::srgb,
    };
    gfx::image_format depth_format = gfx::image_format::d32_float;

    m_renderpass = m_context.get_resources().create_renderpass({
        .color_attachment_formats = nstl::array{ surface_format.format },
        .depth_stencil_attachment_format = depth_format,

        .has_presentable_images = true,
        .keep_depth_values_after_renderpass = false,
    });

    m_context.get_instance().set_debug_name(m_context.get_resources().get_renderpass(m_renderpass).get_handle(), "Main renderpass");

    m_swapchain = nstl::make_unique<swapchain>(m_context, m_renderpass, surface_format, depth_format, w, h);
}

void gfx_vk::renderer::create_frame_resources(renderer_config const& config)
{
    for (size_t i = 0; i < config.max_frames_in_flight; i++)
    {
        command_pool pool{ m_context, m_context.get_instance().get_graphics_queue_family_index() };
        VkCommandBuffer command_buffer = pool.allocate();

        m_frame_resources.push_back({
            .image_available_semaphore{ m_context },
            .render_finished_semaphore{ m_context },
            .in_flight_fence{ m_context },
            .command_pool = nstl::move(pool),
            .command_buffer = command_buffer,
        });

        frame_resources& resources = m_frame_resources.back();

        m_context.get_instance().set_debug_name(resources.image_available_semaphore.get_handle(), "Image available semaphore (frame {})", i);
        m_context.get_instance().set_debug_name(resources.render_finished_semaphore.get_handle(), "Render finished semaphore (frame {})", i);
        m_context.get_instance().set_debug_name(resources.in_flight_fence.get_handle(), "In-flight fence (frame {})", i);
        m_context.get_instance().set_debug_name(resources.command_pool.get_handle(), "Main command pool (frame {})", i);
        m_context.get_instance().set_debug_name(resources.command_buffer, "Main command buffer (frame {})", i);
    }
}

gfx_vk::renderer::frame_resources& gfx_vk::renderer::get_current_frame_resources()
{
    return m_frame_resources[m_context.get_mutable_resource_index()];
}
