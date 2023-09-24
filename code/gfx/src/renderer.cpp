#include "gfx/renderer.h"

void gfx::renderer::set_backend(nstl::unique_ptr<backend> backend)
{
    m_backend = nstl::move(backend);
}
