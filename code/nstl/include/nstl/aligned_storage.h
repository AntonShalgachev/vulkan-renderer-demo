#pragma once

namespace nstl
{
    template<size_t Size, size_t Alignment>
    struct aligned_storage_t
    {
        alignas(Alignment) unsigned char data[Size];
    };
}
