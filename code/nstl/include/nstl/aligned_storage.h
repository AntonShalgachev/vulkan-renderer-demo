#pragma once

#include "nstl/new.h"
#include "nstl/utility.h"

namespace nstl
{
    template<typename T, size_t Size, size_t Alignment>
    concept matches_size_alignment = sizeof(T) <= Size && alignof(T) <= Alignment;

    template<size_t Size, size_t Alignment>
    struct aligned_storage_t
    {
        alignas(Alignment) unsigned char data[Size];

        template<typename T, typename... Args>
        requires matches_size_alignment<T, Size, Alignment>
        static aligned_storage_t<Size, Alignment> create(Args&&... args)
        {
            aligned_storage_t<Size, Alignment> result;
            new(nstl::new_tag{}, &result.data) T{ nstl::forward<Args>(args)... };
            return result;
        }

        template<typename T, typename... Args>
        requires matches_size_alignment<T, Size, Alignment>
        void create_inplace(Args&&... args)
        {
            new(nstl::new_tag{}, &data) T{ nstl::forward<Args>(args)... };
        }

        template<typename T>
        requires matches_size_alignment<T, Size, Alignment>
        void destroy()
        {
            get_as<T>().~T();
        }

        template<typename T>
        requires matches_size_alignment<T, Size, Alignment>
        T& get_as()
        {
            return *reinterpret_cast<T*>(&data);
        }

        template<typename T>
        requires matches_size_alignment<T, Size, Alignment>
        T const& get_as() const
        {
            return *reinterpret_cast<T const*>(&data);
        }
    };
}
