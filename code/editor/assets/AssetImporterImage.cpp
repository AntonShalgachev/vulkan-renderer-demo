#include "AssetImporterImage.h"

#include "AssetDatabase.h"
#include "ImportDescription.h"

#include "path/path.h"

#include "common/Utils.h"

#include "nstl/string.h"
#include "nstl/blob_view.h"

#include "tiny_ktx/tiny_ktx.h"

#include "stb_image.h"

#include <vulkan/vulkan.h> // TODO remove?

namespace
{
    nstl::vector<unsigned char> convertToKtx2(nstl::span<unsigned char const> content, size_t width, size_t height, int comp)
    {
        auto getFormat = [](int comp)
        {
            if (comp == 3)
                return VK_FORMAT_R8G8B8_UNORM;
            if (comp == 4)
                return VK_FORMAT_R8G8B8A8_UNORM;

            assert(false);
            return VK_FORMAT_UNDEFINED;
        };

        class memory_stream : public tiny_ktx::output_stream
        {
        public:
            memory_stream(nstl::vector<unsigned char>& bytes) : m_bytes(bytes) {}

            bool write(void const* src, size_t size) override
            {
                if (!src)
                    return false;

                size_t destinationOffset = m_bytes.size();
                m_bytes.resize(m_bytes.size() + size);
                memcpy(m_bytes.data() + destinationOffset, src, size);

                return true;
            }

        private:
            nstl::vector<unsigned char>& m_bytes;
        };

        tiny_ktx::image_level_info info{
            .byte_offset = 0,
            .byte_length = content.size(),
            .uncompressed_byte_length = content.size(),
        };

        tiny_ktx::image_parameters params = {
            .vk_format = static_cast<uint32_t>(getFormat(comp)),
            .pixel_width = static_cast<uint32_t>(width),
            .pixel_height = static_cast<uint32_t>(height),

            .level_infos = &info,
            .levels_count = 1,

            .data = content.data(),
            .data_size = content.size(),
        };

        nstl::vector<unsigned char> bytes;
        memory_stream stream{ bytes };
        bool result = tiny_ktx::write_image(params, stream);
        assert(result);

        return bytes;
    }

    nstl::vector<unsigned char> createImage(nstl::blob_view content)
    {
        int req_comp = 4; // TODO remove this? GPU doesn't support RGB format
        int w = 0, h = 0, comp = 0;
        unsigned char* data = stbi_load_from_memory(content.ucdata(), content.size(), &w, &h, &comp, req_comp);
        int bits = 8;

        if (req_comp != 0)
            comp = req_comp;

        assert(data);
        assert(w > 0 && h > 0);
        assert(comp > 0);

        size_t dataSize = static_cast<size_t>(w * h * comp) * size_t(bits / 8);

        nstl::vector<unsigned char> bytes = convertToKtx2({ data, dataSize }, w, h, comp);

        stbi_image_free(data);

        return bytes;
    }
}

editor::assets::AssetImporterImage::AssetImporterImage(AssetDatabase& database) : m_database(database)
{

}

nstl::vector<editor::assets::Uuid> editor::assets::AssetImporterImage::importAsset(ImportDescription const& desc) const
{
    Uuid id = m_database.createAsset(AssetType::Image, desc.name);

    nstl::vector<unsigned char> bytes = createImage(desc.content);
    nstl::string filename = "texture.ktx2";
    m_database.addAssetFile(id, bytes, filename);

    return { id };
}
