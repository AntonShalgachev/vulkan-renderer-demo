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
