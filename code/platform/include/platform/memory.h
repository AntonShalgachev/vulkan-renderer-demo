#pragma once

namespace platform
{
    void* allocate(size_t size);
    void deallocate(void* ptr);
}
