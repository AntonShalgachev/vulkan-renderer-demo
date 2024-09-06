#pragma once

#include "nstl/function.h"
#include "nstl/scope_exit.h"

#include "platform/threading.h"

namespace mt
{
    template<typename FuncPtrType>
    void thread_func_typed(void* param)
    {
        auto func = static_cast<FuncPtrType>(param);
        assert(func);
        nstl::scope_exit on_exit = [func]() { delete func; };

        (*func)();
    }

    class thread
    {
    public:
        template<typename F, typename... T>
        thread(nstl::string_view name, F&& f, T&&... args)
        {
            // func ownership is transferred to the thread
            auto func = new auto ([f = nstl::forward<F>(f), ...args = nstl::forward<T>(args)]() { f(args...); });

            bool res = platform::thread_create(m_storage, thread_func_typed<decltype(func)>, func, name);
            if (!res)
            {
                // ownership hasn't been transferred => clean-up ourselves
                delete func;
                assert(false);
            }
        }

        thread(thread&& rhs);
        thread& operator=(thread&& rhs);

        ~thread();

        void join();

    private:
        platform::thread_storage_t m_storage;
    };
}
