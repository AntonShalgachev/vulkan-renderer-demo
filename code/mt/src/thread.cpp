#include "mt/thread.h"

static void thread_main(void* arg)
{
    
}

mt::thread::thread()
{
    platform::create_thread(thread_main, nullptr);
}

void mt::thread::join()
{
    // TODO implement
}
