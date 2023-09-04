#pragma once

#include "tiny_ktx/stream.h"

#include "stdint.h"

namespace tiny_ktx
{
    struct uint96_t
    {
        uint8_t bytes[12];

        bool operator==(uint96_t const&) const = default;
    };

    inline constexpr uint96_t KTX_IDENTIFIER = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};

    struct image_header
    {
        uint96_t identifier = KTX_IDENTIFIER;

        uint32_t vk_format = 0;
        uint32_t type_size = 0;
        uint32_t pixel_width = 0;
        uint32_t pixel_height = 0;
        uint32_t pixel_depth = 0;
        uint32_t layer_count = 0;
        uint32_t face_count = 0;
        uint32_t level_count = 0;
        uint32_t supercompression_scheme = 0;

        // data format descriptor
        uint32_t dfd_byte_offset = 0;
        uint32_t dfd_byte_length = 0;

        // key-value data
        uint32_t kvd_byte_offset = 0;
        uint32_t kvd_byte_length = 0;

        // supercompression global data
        uint64_t sgd_byte_offset = 0;
        uint64_t sgd_byte_length = 0;
    };

    static_assert(sizeof(image_header) == 80);

    [[nodiscard]] bool parse_header(image_header* header, input_stream& stream);

    struct image_level_info
    {
        uint64_t byte_offset;
        uint64_t byte_length;
        uint64_t uncompressed_byte_length;
    };

    [[nodiscard]] size_t get_level_count(image_header const& header);
    [[nodiscard]] bool load_image_level_index(image_level_info* infos, size_t count, image_header const& header, input_stream& stream);

    [[nodiscard]] bool write_image(image_header& header, image_level_info* infos, size_t count, void const* data, size_t data_size, output_stream& stream); // TODO header & infos should be const
}
