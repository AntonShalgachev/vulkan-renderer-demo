#pragma once

#include "gfx/backend.h"
#include "gfx/resources.h"

#include "nstl/span.h"
#include "nstl/unique_ptr.h"

#include <stdint.h>

namespace gfx
{
    class buffer;
    class image;
    class sampler;
    class renderpass;
    class framebuffer;
    class descriptorgroup;
    class shader;
    class renderstate;

    //////////////////////////////////////////////////////////////////////////
    // Expectations:
    // * begin_resource_update is called exactly once per frame before any resource updates
    // * begin_frame is called exactly once per frame before any draw commands
    // * acquire_main_framebuffer is called exactly once per frame
    // * renderpass_begin/renderpass_end with the main framebuffer is called exactly once per frame
    // * if a resource is mutable, then it should be fully updated each frame it's used (no incremental frame-to-frame modifications, no skipping frames)
    // * Images that are uploaded and sampled should have usage "upload_sampled"
    // * attribute_description::buffer_binding_index is a valid index into vertex_configuration_view::buffer_bindings
    // * attribute_description::location is unique
    //////////////////////////////////////////////////////////////////////////

    class renderer
    {
    public:
        renderer(nstl::unique_ptr<backend> backend);
        void set_backend(nstl::unique_ptr<backend> backend);

        void resize_main_framebuffer(tglm::ivec2 size) { return m_backend->resize_main_framebuffer(size); }

        // TODO add some basic validation before calling backend

        // Resource creation/destruction
        [[nodiscard]] buffer_handle create_buffer(buffer_params const& params) { return m_backend->create_buffer(params); }
        [[nodiscard]] image_handle create_image(image_params const& params) { return m_backend->create_image(params); }
        [[nodiscard]] sampler_handle create_sampler(sampler_params const& params) { return m_backend->create_sampler(params); }
        [[nodiscard]] renderpass_handle create_renderpass(renderpass_params const& params) { return m_backend->create_renderpass(params); }
        [[nodiscard]] framebuffer_handle create_framebuffer(framebuffer_params const& params) { return m_backend->create_framebuffer(params); }
        [[nodiscard]] descriptorgroup_handle create_descriptorgroup(descriptorgroup_params const& params) { return m_backend->create_descriptorgroup(params); }
        [[nodiscard]] shader_handle create_shader(shader_params const& params) { return m_backend->create_shader(params); }
        [[nodiscard]] renderstate_handle create_renderstate(renderstate_params const& params) { return m_backend->create_renderstate(params); }

        // Resource update
        void begin_resource_update() { return m_backend->begin_resource_update(); }
        void buffer_upload_sync(buffer_handle handle, gfx::data_reader& reader, size_t offset = 0) { return m_backend->buffer_upload_sync(handle, reader, offset); } // TODO: add async upload
        void buffer_upload_sync(buffer_handle handle, nstl::blob_view bytes, size_t offset = 0);
        void image_upload_sync(image_handle handle, data_reader& reader) { return m_backend->image_upload_sync(handle, reader); } // TODO: add async upload
        void image_upload_sync(image_handle handle, nstl::blob_view bytes);

        // Main framebuffer resources
        [[nodiscard]] renderpass_handle get_main_renderpass() { return m_backend->get_main_renderpass(); }
        [[nodiscard]] framebuffer_handle acquire_main_framebuffer() { return m_backend->acquire_main_framebuffer(); }
        [[nodiscard]] float get_main_framebuffer_aspect() { return m_backend->get_main_framebuffer_aspect(); }

        // Command submission
        void begin_frame() { return m_backend->begin_frame(); }

        void renderpass_begin(renderpass_begin_params const& params) { return m_backend->renderpass_begin(params); }
        void renderpass_end() { return m_backend->renderpass_end(); }

        void draw_indexed(draw_indexed_args const& args) { return m_backend->draw_indexed(args); }

        void submit() { return m_backend->submit(); }

    private:
        nstl::unique_ptr<backend> m_backend;
    };
}
