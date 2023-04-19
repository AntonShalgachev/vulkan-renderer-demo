#include "platform/threading.h"

#include "windows_common.h"
#include "platform/memory.h"

#include "Windows.h"

#include <assert.h>

namespace
{
    struct new_tag {};

    struct thread_args
    {
        platform::thread_func_t func = nullptr;
        void* arg = nullptr;
    };
}

void* operator new(size_t size, ::new_tag, void* p) { return p; }
void operator delete(void*, ::new_tag, void*) {}

static DWORD WINAPI win32_thread_func(void* param)
{
    thread_args* args = static_cast<thread_args*>(param);
    thread_args args_copy = *args;

    args->~thread_args();
    platform::deallocate(param);
    param = nullptr;
    args = nullptr;

    args_copy.func(args_copy.arg);

    return 0;
}

nstl::optional<platform::thread_handle> platform::create_thread(thread_func_t func, void* arg)
{
    DWORD creationFlags = 0;
    SIZE_T stackSize = 0; // default

    // TODO what if the thread is valid but would never start? In this case `args` would leak
    void* ptr = platform::allocate(sizeof(thread_args));
    thread_args* args = new(new_tag{}, ptr) thread_args{ .func = func, .arg = arg };
    assert(args == ptr);

    DWORD id = 0;
    HANDLE h = CreateThread(nullptr, stackSize, win32_thread_func, ptr, creationFlags, &id);

    if (h == nullptr)
    {
        platform::deallocate(ptr);
        auto e = platform_win32::get_last_error(); // TODO make use of it
        assert(false);
        return {};
    }

    return platform_win32::create_handle(h);
}