#pragma once

#include "nstl/utility.h"

namespace nstl
{
    template<typename F>
    class scope_exit
    {
    public:
        scope_exit(F func) : m_func(nstl::move(func)) {}
        ~scope_exit()
        {
            m_func();
        }

    private:
        F m_func;
    };

    template<typename F> scope_exit(F) -> scope_exit<F>;
}
