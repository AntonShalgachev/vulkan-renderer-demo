#pragma once

#include "renderer/vkgfx/Handles.h"

#include "glm.h"

#include "nstl/vector.h"

namespace vkgfx
{
    // TODO
    struct TestObject
    {
        vkgfx::PipelineHandle pipeline;

        vkgfx::MeshHandle mesh;
        vkgfx::MaterialHandle material;

        vkgfx::BufferHandle uniformBuffer;
        nstl::vector<unsigned char> pushConstants; // TODO have a reference to the buffer instead

        bool hasScissors = false;
        glm::uvec2 scissorOffset = { 0, 0 };
        glm::uvec2 scissorSize = { 0, 0 };
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
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 color = glm::vec3(1.0f);
        float intensity = 1.0f;
    };
}
