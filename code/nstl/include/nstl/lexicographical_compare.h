#pragma once

#include <compare> // TODO avoid including this header?

namespace nstl
{
    template<typename It1, typename It2>
    auto lexicographical_compare_three_way(It1 begin1, It1 end1, It2 begin2, It2 end2)
    {
        It1 it1 = begin1;
        It2 it2 = begin2;

        while (it1 != end1)
        {
            if (it2 == end2)
                return std::strong_ordering::greater;
            if (auto result = (*it1 <=> *it2); result != 0)
                return result;
            ++it1;
            ++it2;
        }

        return (it2 == end2) <=> true;
    }
}
