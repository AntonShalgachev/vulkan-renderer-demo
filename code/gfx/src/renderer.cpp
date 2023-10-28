#include "gfx/renderer.h"

gfx::renderer::renderer(nstl::unique_ptr<backend> backend)
{
    set_backend(nstl::move(backend));
}

void gfx::renderer::set_backend(nstl::unique_ptr<backend> backend)
{
    assert(!m_backend); // TODO implement changing backend at runtime

    m_backend = nstl::move(backend);
}

void gfx::renderer::buffer_upload_sync(buffer_handle handle, nstl::blob_view bytes, size_t offset)
{
    memory_reader reader{ bytes };
    return buffer_upload_sync(handle, reader, offset);
}

void gfx::renderer::image_upload_sync(image_handle handle, nstl::blob_view bytes)
{
    memory_reader reader{ bytes };
    return image_upload_sync(handle, reader);
}
