#include "AssetImporterImage.h"

#include "AssetDatabase.h"
#include "ImportDescription.h"

#include "path/path.h"

#include "common/Utils.h"

#include "nstl/string.h"
#include "nstl/blob_view.h"

#include "stb_image.h"
#include "ktx.h"

#include <vulkan/vulkan.h> // TODO remove?

namespace
{
    class KtxMemoryStream : public ktxStream
    {
    public:
        KtxMemoryStream(nstl::vector<unsigned char>& bytes) : m_bytes(bytes)
        {
            ktxStream::type = eStreamTypeCustom;
            ktxStream::data.custom_ptr.address = this;
            ktxStream::readpos = 0;
            ktxStream::write = &write;
            ktxStream::getpos = &getpos;
        }

    private:
        static KTX_error_code write(ktxStream* str, const void* src, const ktx_size_t size, const ktx_size_t count)
        {
            if (!str || !src)
                return KTX_INVALID_VALUE;

            assert(str->type == eStreamTypeCustom);
            assert(str->data.custom_ptr.address);
            KtxMemoryStream const& self = *static_cast<KtxMemoryStream const*>(str->data.custom_ptr.address);

            assert(size == 1 || count == 1);

            size_t destinationOffset = self.m_bytes.size();
            self.m_bytes.resize(self.m_bytes.size() + count * size);
            memcpy(self.m_bytes.data() + destinationOffset, src, count * size);

            return KTX_SUCCESS;
        }

        static KTX_error_code getpos(ktxStream* str, ktx_off_t* pos)
        {
            if (!str || !pos)
                return KTX_INVALID_VALUE;

            assert(str->type == eStreamTypeCustom);
            assert(str->data.custom_ptr.address);
            KtxMemoryStream const& self = *static_cast<KtxMemoryStream const*>(str->data.custom_ptr.address);

            *pos = self.m_bytes.size();

            return KTX_SUCCESS;
        }

    private:
        nstl::vector<unsigned char>& m_bytes;
    };

    nstl::vector<unsigned char> convertToKtx2(nstl::span<unsigned char const> content, size_t width, size_t height, int comp)
    {
        ktxTexture2* texture;
        ktxTextureCreateInfo createInfo;
        KTX_error_code result;
        ktx_uint32_t level, layer, faceSlice;

        auto getFormat = [](int comp)
        {
            if (comp == 3)
                return VK_FORMAT_R8G8B8_UNORM;
            if (comp == 4)
                return VK_FORMAT_R8G8B8A8_UNORM;

            assert(false);
            return VK_FORMAT_UNDEFINED;
        };

        createInfo.vkFormat = getFormat(comp);
        createInfo.baseWidth = width;
        createInfo.baseHeight = height;
        createInfo.baseDepth = 1;
        createInfo.numDimensions = 2;
        createInfo.numLevels = 1;
        createInfo.numLayers = 1;
        createInfo.numFaces = 1;
        createInfo.isArray = false;
        createInfo.generateMipmaps = false;

        result = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
        assert(result == KTX_SUCCESS);

        auto imageSize = ktxTexture_GetImageSize(ktxTexture(texture), 0);
        assert(imageSize == content.size());

        result = ktxTexture_SetImageFromMemory(ktxTexture(texture), 0, 0, 0, content.data(), content.size());
        assert(result == KTX_SUCCESS);

        nstl::vector<unsigned char> bytes;
        KtxMemoryStream stream{ bytes };
        result = ktxTexture_WriteToStream(ktxTexture(texture), &stream);
        assert(result == KTX_SUCCESS);

        ktxTexture_Destroy(ktxTexture(texture));

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
