#include "platform/threading.h"

#include "windows_common.h"

#include "windows_common.h"
#include "platform/memory.h"

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

void* operator new(size_t, ::new_tag, void* p) { return p; }
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

bool platform::create_thread(thread_storage_t& storage, thread_func_t func, void* arg)
{
    DWORD creationFlags = 0;
    SIZE_T stackSize = 0; // default

    // TODO what if the thread is valid but would never start? In this case `args` would leak
    void* ptr = platform::allocate(sizeof(thread_args));
    [[maybe_unused]] thread_args* args = new(new_tag{}, ptr) thread_args{ .func = func, .arg = arg };
    assert(args == ptr);

    DWORD id = 0;
    HANDLE h = CreateThread(nullptr, stackSize, win32_thread_func, ptr, creationFlags, &id);

    if (h == nullptr)
    {
        platform::deallocate(ptr);
        auto e = platform_win32::get_last_error(); // TODO make use of it
        assert(false);
        return false;
    }

    storage.create<HANDLE>(h);
    return true;
}

uint64_t platform::thread_get_current_id()
{
    return GetCurrentThreadId();
}

bool platform::mutex_create(mutex_storage_t& storage)
{
    storage.create<CRITICAL_SECTION>();

    CRITICAL_SECTION& section = storage.get_as<CRITICAL_SECTION>();
    InitializeCriticalSection(&section);

    return true;
}

void platform::mutex_destroy(mutex_storage_t& storage)
{
    storage.destroy<CRITICAL_SECTION>();
}

void platform::mutex_lock(mutex_storage_t& storage)
{
    CRITICAL_SECTION& section = storage.get_as<CRITICAL_SECTION>();
    EnterCriticalSection(&section);
}

void platform::mutex_unlock(mutex_storage_t& storage)
{
    CRITICAL_SECTION& section = storage.get_as<CRITICAL_SECTION>();
    LeaveCriticalSection(&section);
}
