#include "framework.h"

// stb_image and tiny_obj_loader produce this warning
#pragma warning(disable:5045)

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "ServiceLocator.h"
#include "Renderer.h"

namespace vkr
{
    namespace temp
    {
        std::unique_ptr<Renderer> const& getRenderer()
        {
            return vkr::ServiceLocator::instance().getRenderer();
        }

        VkDevice getDevice()
        {
            return getRenderer()->getDevice();
        }
    }
}
