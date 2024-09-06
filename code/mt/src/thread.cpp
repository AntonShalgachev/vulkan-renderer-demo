#include "mt/thread.h"

#include "nstl/string_view.h"

#include <assert.h>

mt::thread::thread(thread&& rhs)
{
    bool res = platform::thread_create_empty(m_storage);
    assert(res);

    *this = nstl::move(rhs);
}

mt::thread& mt::thread::operator=(thread&& rhs)
{
    platform::thread_swap(m_storage, rhs.m_storage);
    return *this;
}

mt::thread::~thread()
{
    platform::thread_destroy(m_storage);
}

void mt::thread::join()
{
    platform::thread_join(m_storage);
}
