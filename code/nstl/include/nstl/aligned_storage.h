#pragma once

#include "nstl/new.h"
#include "nstl/utility.h"

namespace nstl
{
    template<size_t Size, size_t Alignment>
    struct aligned_storage_t
    {
        alignas(Alignment) unsigned char data[Size];

        template<typename T, typename... Args>
        void create(Args&&... args)
        {
            new(nstl::new_tag{}, &data) T{ nstl::forward<Args>(args)... };
        }

        template<typename T>
        void destroy()
        {
            get_as<T>().~T();
        }

        template<typename T>
        T& get_as()
        {
            static_assert(Size >= sizeof(T));
            static_assert(Alignment >= alignof(T));

            return *reinterpret_cast<T*>(&data);
        }

        template<typename T>
        T const& get_as() const
        {
            static_assert(Size >= sizeof(T));
            static_assert(Alignment >= alignof(T));

            return *reinterpret_cast<T const*>(&data);
        }
    };
}
