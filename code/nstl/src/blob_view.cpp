#include "nstl/blob_view.h"

nstl::blob_view nstl::blob_view::subview(size_t offset, size_t count) const
{
    NSTL_ASSERT(offset <= m_size);

    if (count > m_size - offset)
        count = m_size - offset;

    NSTL_ASSERT(count <= m_size);
    NSTL_ASSERT(offset + count <= m_size);
    return { static_cast<char const*>(m_data) + offset, count };
}
