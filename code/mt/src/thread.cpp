#include "mt/thread.h"

#include <assert.h>

static void thread_main(void*)
{
    
}

mt::thread::thread()
{
    [[maybe_unused]] bool res = platform::create_thread(m_storage, thread_main, nullptr);
    assert(res);
}

void mt::thread::join()
{
    // TODO implement
}
