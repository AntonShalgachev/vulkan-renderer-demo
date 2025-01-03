#include "platform/threading.h"

#include "common.h"

#include "platform/memory.h"

#include "nstl/string_view.h"

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

bool platform::thread_create_empty(thread_storage_t& storage)
{
    storage.create_inplace<HANDLE>(INVALID_HANDLE_VALUE);
    return true;
}

bool platform::thread_create(thread_storage_t& storage, thread_func_t func, void* arg, nstl::string_view name)
{
    DWORD creationFlags = 0;
    SIZE_T stackSize = 0; // default

    void* ptr = platform::allocate(sizeof(thread_args), alignof(thread_args));
    [[maybe_unused]] thread_args* args = new(new_tag{}, ptr) thread_args{ .func = func, .arg = arg };
    assert(args == ptr);

    DWORD id = 0;
    HANDLE h = CreateThread(nullptr, stackSize, win32_thread_func, ptr, creationFlags, &id);

    if (!name.empty())
    {
        platform_win64::wstring wname = platform_win64::convert_to_wstring(name);
        SetThreadDescription(h, wname.c_str());
    }

    if (h == nullptr)
    {
        platform::deallocate(ptr);
        auto e = platform_win64::get_last_error(); // TODO make use of it
        assert(false);
        return false;
    }

    storage.create_inplace<HANDLE>(h);
    return true;
}

void platform::thread_swap(thread_storage_t& lhs, thread_storage_t& rhs)
{
    nstl::exchange(lhs.get_as<HANDLE>(), rhs.get_as<HANDLE>());
}

void platform::thread_join(thread_storage_t& storage)
{
    HANDLE h = storage.get_as<HANDLE>();
    if (h != INVALID_HANDLE_VALUE)
        WaitForSingleObject(h, INFINITE);
}

void platform::thread_destroy(thread_storage_t& storage)
{
    HANDLE h = storage.get_as<HANDLE>();
    if (h != INVALID_HANDLE_VALUE)
    {
        WaitForSingleObject(h, INFINITE);
        CloseHandle(h);
    }
}

uint64_t platform::thread_get_current_id()
{
    return GetCurrentThreadId();
}

void platform::sleep(uint64_t milliseconds)
{
    static_assert(sizeof(DWORD) == sizeof(uint32_t));
    assert(milliseconds <= UINT32_MAX);
    ::Sleep(static_cast<DWORD>(milliseconds));
}

bool platform::mutex_create(mutex_storage_t& storage)
{
    storage.create_inplace<CRITICAL_SECTION>();

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
