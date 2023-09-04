#include "tiny_ktx/tiny_ktx.h"

#include "string.h"

namespace
{
    bool check_header(tiny_ktx::image_header const& header)
    {
        static_assert(sizeof(header.identifier) == sizeof(tiny_ktx::KTX_IDENTIFIER));
        if (header.identifier != tiny_ktx::KTX_IDENTIFIER)
            return false;

        // TODO further checks

        return true;
    }

    // TODO temporary hardcoded block. Remove
    constexpr uint8_t FORMAT_DESCRIPTION[] = {
        0x5c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x58, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x08, 0x00,
        0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x10, 0x00, 0x07, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x18, 0x00, 0x07, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
    };
    static_assert(sizeof(FORMAT_DESCRIPTION) == 92);
}

bool tiny_ktx::parse_header(image_header* header, input_stream& stream)
{
    image_header h;
    if (!stream.read(&h, sizeof(h)))
        return false;

    if (!check_header(h))
        return false;

    *header = h;
    return true;
}

size_t tiny_ktx::get_level_count(image_header const& header)
{
    return header.level_count > 0 ? header.level_count : 1;
}

bool tiny_ktx::load_image_level_index(image_level_info* infos, size_t count, image_header const& header, input_stream& stream)
{
    if (count > get_level_count(header))
        return false;

    size_t byte_size = sizeof(image_level_info) * count;
    if (!stream.read(infos, byte_size))
        return false;

    return true;
}

bool tiny_ktx::write_image(image_header& header, image_level_info* infos, size_t count, void const* data, size_t data_size, output_stream& stream)
{
    size_t index_byte_size = sizeof(image_level_info) * count;

    size_t dfd_offset = sizeof(header) + index_byte_size;
    size_t data_offset = dfd_offset + sizeof(FORMAT_DESCRIPTION);

    header.dfd_byte_offset = dfd_offset;
    header.dfd_byte_length = sizeof(FORMAT_DESCRIPTION);

    for (size_t i = 0; i < count; i++)
        infos[i].byte_offset += data_offset;

    if (!stream.write(&header, sizeof(header)))
        return false;

    if (!stream.write(infos, index_byte_size))
        return false;

    if (!stream.write(&FORMAT_DESCRIPTION, sizeof(FORMAT_DESCRIPTION)))
        return false;

    if (!stream.write(data, data_size))
        return false;

    return true;
}
