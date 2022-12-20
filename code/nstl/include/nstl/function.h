#pragma once

#include "nstl/assert.h"
#include "nstl/utility.h"

#include <stddef.h>

namespace nstl
{
    template<typename Signature>
    class function;

    template<typename R, typename... Args>
    class function<R(Args...)>
    {
    public:
        function(nullptr_t = nullptr) {}

        template<typename Func>
        function(Func func)
        {
            m_func = new Func(nstl::move(func));
            m_callFunc = [](void* func, Args... args) -> R {
                return (*static_cast<Func*>(func))(nstl::move(args)...);
            };
            m_destroyFunc = [](void* func) {
                delete static_cast<Func*>(func);
            };
        }

        function(function const& rhs) = delete;
        function(function&& rhs)
        {
            swap(rhs);
        }

        ~function()
        {
            if (m_func && m_destroyFunc)
                m_destroyFunc(m_func);
        }

        function& operator=(function const& rhs) = delete;
        function& operator=(function&& rhs)
        {
            swap(rhs);
            return *this;
        }

        R operator()(Args... args) const
        {
            NSTL_ASSERT(m_func);
            NSTL_ASSERT(m_callFunc);
            return m_callFunc(m_func, nstl::move(args)...);
        }

        operator bool() const
        {
            return m_func;
        }

    private:
        void swap(function& rhs)
        {
            nstl::exchange(m_func, rhs.m_func);
            nstl::exchange(m_callFunc, rhs.m_callFunc);
            nstl::exchange(m_destroyFunc, rhs.m_destroyFunc);
        }

        using CallFuncPtr = R(*)(void* func, Args... args);
        using DestroyFuncPtr = void(*)(void* func);

        void* m_func = nullptr;
        CallFuncPtr m_callFunc = nullptr;
        DestroyFuncPtr m_destroyFunc = nullptr;
    };
}
