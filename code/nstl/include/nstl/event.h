#pragma once

#include "nstl/function.h"
#include "nstl/vector.h"

namespace nstl
{
    template<typename... Args>
    class event
    {
    public:
        void add(nstl::function<void(Args...)> delegate)
        {
            m_delegates.push_back(nstl::move(delegate));
        }

        void operator()(Args const&... args)
        {
            for (auto const& delegate : m_delegates)
                if (delegate)
                    delegate(args...);
        }

    private:
        nstl::vector<nstl::function<void(Args...)>> m_delegates;
    };
}
