#pragma once

namespace tiny_ktx
{
    struct input_stream
    {
        [[nodiscard]] virtual bool read(void* dest, size_t size) = 0;
    };

    struct output_stream
    {
        [[nodiscard]] virtual bool write(void const* src, size_t size) = 0;
    };
}
