#pragma once

#include "vkgfx/Handles.h"

#include "gfx/resources.h"

#include "tglm/types.h"

#include "nstl/static_vector.h"

namespace vkgfx
{
    constexpr size_t MaxPushConstantsSize = 64;

    // TODO
    struct TestObject
    {
        vkgfx::PipelineHandle pipeline;
        vkgfx::PipelineHandle shadowmapPipeline;

        gfx::renderstate_handle state;
        gfx::renderstate_handle shadowmapState;

        vkgfx::MeshHandle mesh;
        vkgfx::MaterialHandle material;
        gfx::descriptorgroup_handle materialDescriptorGroup;

        gfx::buffer_handle newUniformBuffer;
        gfx::descriptorgroup_handle descriptorGroup;

        nstl::vector<gfx::buffer_with_offset> vertexBuffers;
        gfx::buffer_with_offset indexBuffer;
        gfx::index_type indexType = gfx::index_type::uint16;
        size_t indexCount = 0;

        vkgfx::BufferHandle uniformBuffer;
        nstl::static_vector<unsigned char, MaxPushConstantsSize> pushConstants; // TODO have a reference to the buffer instead

        bool hasScissors = false;
        tglm::ivec2 scissorOffset = { 0, 0 };
        tglm::ivec2 scissorSize = { 0, 0 };
    };

    struct TestCameraTransform
    {
        tglm::vec3 position = { 0.0f, 0.0f, 0.0f };
        tglm::quat rotation = tglm::quat::identity();
    };

    struct TestCameraParameters
    {
        float fov = 45.0f;
        float nearZ = 0.1f;
        float farZ = 10000.0f;
    };

    struct TestLightParameters
    {
        tglm::vec3 position = { 0.0f, 0.0f, 0.0f };
        tglm::vec3 color = { 1.0f, 1.0f, 1.0f };
        float intensity = 1.0f;
    };
}
