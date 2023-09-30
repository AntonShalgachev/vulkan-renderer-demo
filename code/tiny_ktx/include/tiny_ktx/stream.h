#pragma once

namespace tiny_ktx
{
    struct input_stream
    {
        virtual ~input_stream() = default;
        [[nodiscard]] virtual bool read(void* dest, size_t size) = 0;
    };

    struct output_stream
    {
        virtual ~output_stream() = default;
        [[nodiscard]] virtual bool write(void const* src, size_t size) = 0;
    };
}
