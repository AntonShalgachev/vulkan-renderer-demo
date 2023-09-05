#pragma once

#include <stdlib.h>

namespace nstl
{
    template<typename InputIt, typename Pred>
    void simple_sort(InputIt begin, InputIt end, Pred&& pred)
    {
        using T = nstl::simple_decay_t<decltype(*begin)>;
        using PredPtr = decltype(&pred);

        auto comp = [](void* context, void const* lhs, void const* rhs)
        {
            auto&& pred = *static_cast<PredPtr>(context);
            T const& l = *static_cast<T const*>(lhs);
            T const& r = *static_cast<T const*>(rhs);

            if (pred(l, r))
                return -1;
            if (pred(r, l))
                return 1;
            return 0;
        };

        qsort_s(&*begin, static_cast<size_t>(end - begin), sizeof(T), comp, &pred);
    }
}