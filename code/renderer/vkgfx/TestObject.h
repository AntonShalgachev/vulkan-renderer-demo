#pragma once

#include "Handles.h"
#include "glm.h"

#include <vector>

namespace vkgfx
{
    // TODO
    struct TestObject
    {
        vkgfx::MeshHandle mesh;
        vkgfx::MaterialHandle material;

        vkgfx::PipelineHandle pipeline;

        vkgfx::BufferHandle uniformBuffer;
        std::vector<std::byte> pushConstants; // TODO don't use dynamic memory
    };

    struct TestCameraTransform
    {
        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::identity<glm::quat>();
    };

    struct TestCameraParameters
    {
        float fov = 45.0f;
        float nearZ = 0.1f;
        float farZ = 10000.0f;
    };

    struct TestLightParameters
    {
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
    };
}
