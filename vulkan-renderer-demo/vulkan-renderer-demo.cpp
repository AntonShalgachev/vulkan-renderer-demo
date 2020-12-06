#include "framework.h"
#include "Renderer.h"
#include "ServiceLocator.h"
#include "ImageView.h"
#include "DeviceMemory.h"
#include "Image.h"
#include "Buffer.h"
#include "Sampler.h"
#include "ShaderModule.h"
#include "Swapchain.h"
#include <memory>
#include "RenderPass.h"
#include "Framebuffer.h"

namespace
{
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription{};

            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            return attributeDescriptions;
        }

        bool operator==(const Vertex& other) const
        {
            return pos == other.pos && color == other.color && texCoord == other.texCoord;
        }
    };

    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    const uint32_t TARGET_WINDOW_WIDTH = 800;
    const uint32_t TARGET_WINDOW_HEIGHT = 600;

    const std::string MODEL_PATH = "data/models/viking_room.obj";
    const std::string TEXTURE_PATH = "data/textures/viking_room.png";

    const int MAX_FRAMES_IN_FLIGHT = 2;
}

namespace std
{
    template<> struct hash<Vertex>
    {
        size_t operator()(Vertex const& vertex) const
        {
            auto pos = hash<glm::vec3>()(vertex.pos);
            auto color = hash<glm::vec3>()(vertex.color);
            auto texCoord = hash<glm::vec2>()(vertex.texCoord);

            return ((pos ^ (color << 1)) >> 1) ^ (texCoord << 1);
        }
    };
}

class HelloTriangleApplication
{
public:
    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_window = glfwCreateWindow(TARGET_WINDOW_WIDTH, TARGET_WINDOW_HEIGHT, "Vulkan Demo", nullptr, nullptr);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    }

    static HelloTriangleApplication* getAppFromWindow(GLFWwindow* window)
    {
        return reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        if (auto app = getAppFromWindow(window))
            app->onFramebufferResized(width, height);
    }

    void onFramebufferResized(int width, int height)
    {
        m_framebufferResized = true;

        getRenderer()->OnSurfaceChanged(width, height);
    }

    void initVulkan()
    {
        auto renderer = std::make_unique<vkr::Renderer>(m_window);

        vkr::ServiceLocator::instance().setRenderer(std::move(renderer));

        createDescriptorSetLayout();
        createTextureImage();
        createTextureImageView();
        createTextureSampler();
        loadModel();
        createVertexBuffer();
        createIndexBuffer();

        initSwapchain();
        createSyncObjects();
    }

    void initSwapchain()
    {
        createSwapchain();
        createRenderPass();
        createGraphicsPipeline();
        createDepthResources();
        createFramebuffers();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
    }

    void recreateSwapchain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(m_window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(getDevice());

        cleanupSwapchain();
        initSwapchain();
    }

    void createSwapchain()
    {
        // TODO research why Vulkan crashes without this line
        m_swapchain = nullptr;

        m_swapchain = std::make_unique<vkr::Swapchain>();
    }

    void createRenderPass()
    {
        m_renderPass = std::make_unique<vkr::RenderPass>(*m_swapchain);
    }

    void createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerLayoutBinding.pImmutableSamplers = nullptr;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        descriptorSetLayoutCreateInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(getDevice(), &descriptorSetLayoutCreateInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
            throw std::runtime_error("failed to create descriptor set layout!");
    }

    void createGraphicsPipeline()
    {
        vkr::ShaderModule vertShaderModule{ "data/shaders/vert.spv" };
        vkr::ShaderModule fragShaderModule{ "data/shaders/frag.spv" };

        VkPipelineShaderStageCreateInfo vertShaderStageInfo = vertShaderModule.createStageCreateInfo(vkr::ShaderModule::Type::Vertex, "main");
        VkPipelineShaderStageCreateInfo fragShaderStageInfo = fragShaderModule.createStageCreateInfo(vkr::ShaderModule::Type::Fragment, "main");

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
        vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
        vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
        vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
        vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

        auto bindingDescription = Vertex::getBindingDescription();
        vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;

        auto attributeDescriptions = Vertex::getAttributeDescriptions();
        vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
        inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

        VkExtent2D swapchainExtent = m_swapchain->getExtent();

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = swapchainExtent.width * 1.0f;
        viewport.height = swapchainExtent.height * 1.0f;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapchainExtent;

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.pViewports = &viewport;
        viewportStateCreateInfo.scissorCount = 1;
        viewportStateCreateInfo.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
        rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizerCreateInfo.depthClampEnable = VK_FALSE;
        rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizerCreateInfo.lineWidth = 1.0f;
        rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
        rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
        rasterizerCreateInfo.depthBiasClamp = 0.0f;
        rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo{};
        multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
        multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisamplingCreateInfo.minSampleShading = 1.0f;
        multisamplingCreateInfo.pSampleMask = nullptr;
        multisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
        multisamplingCreateInfo.alphaToOneEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo{};
        colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendingCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY;
        colorBlendingCreateInfo.attachmentCount = 1;
        colorBlendingCreateInfo.pAttachments = &colorBlendAttachment;
        colorBlendingCreateInfo.blendConstants[0] = 0.0f;
        colorBlendingCreateInfo.blendConstants[1] = 0.0f;
        colorBlendingCreateInfo.blendConstants[2] = 0.0f;
        colorBlendingCreateInfo.blendConstants[3] = 0.0f;

        VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
        depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilCreateInfo.depthTestEnable = VK_TRUE;
        depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
        depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilCreateInfo.stencilTestEnable = VK_FALSE;

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
        pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(getDevice(), &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout!");

        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stageCount = 2;
        pipelineCreateInfo.pStages = shaderStages;
        pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
        pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
        pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
        pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
        pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
        pipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
        pipelineCreateInfo.pDynamicState = nullptr;
        pipelineCreateInfo.layout = m_pipelineLayout;
        pipelineCreateInfo.renderPass = m_renderPass->getHandle();
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineCreateInfo.basePipelineIndex = -1;

        if (vkCreateGraphicsPipelines(getDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_pipeline) != VK_SUCCESS)
            throw std::runtime_error("failed to create graphics pipeline!");
    }

    void createFramebuffers()
    {
        m_swapchain->createFramebuffers(*m_renderPass, *m_depthImageView);
    }

    void createDepthResources()
    {
        VkFormat depthFormat = m_renderPass->getDepthFormat();
        VkExtent2D swapchainExtent = m_swapchain->getExtent();

        createImage(swapchainExtent.width, swapchainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
        m_depthImageView = m_depthImage->createImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    bool hasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void createTextureImage()
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (!pixels)
            throw std::runtime_error("failed to load texture image!");

        size_t imageSize = texWidth * texHeight * 4;

        std::unique_ptr<vkr::Buffer> stagingBuffer;
        std::unique_ptr<vkr::DeviceMemory> stagingBufferMemory;
        createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        stagingBufferMemory->copyFrom(pixels, imageSize);

        stbi_image_free(pixels);

        createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);

        transitionImageLayout(m_textureImage->getHandle(), m_textureImage->getFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        // TODO extract to some class
        copyBufferToImage(stagingBuffer->getHandle(), m_textureImage->getHandle(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        transitionImageLayout(m_textureImage->getHandle(), m_textureImage->getFormat(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void createTextureImageView()
    {
        m_textureImageView = m_textureImage->createImageView(VK_IMAGE_ASPECT_COLOR_BIT);
    }

    void createTextureSampler()
    {
        m_textureSampler = std::make_unique<vkr::Sampler>();
    }

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        endSingleTimeCommands(commandBuffer);
    }

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        endSingleTimeCommands(commandBuffer);
    }

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, std::unique_ptr<vkr::Image>& image, std::unique_ptr<vkr::DeviceMemory>& imageMemory)
    {
        image = std::make_unique<vkr::Image>(width, height, format, tiling, usage);
        imageMemory = std::make_unique<vkr::DeviceMemory>(image->getMemoryRequirements(), properties);
        image->bind(*imageMemory);
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, std::unique_ptr<vkr::Buffer>& buffer, std::unique_ptr<vkr::DeviceMemory>& bufferMemory)
    {
        buffer = std::make_unique<vkr::Buffer>(size, usage);
        bufferMemory = std::make_unique<vkr::DeviceMemory>(buffer->getMemoryRequirements(), properties);
        buffer->bind(*bufferMemory);
    }

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void loadModel()
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warnings, errors;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warnings, &errors, MODEL_PATH.c_str()))
            throw std::runtime_error(warnings + errors);

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
                    m_vertices.push_back(vertex);
                }

                m_indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

    void createVertexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

        std::unique_ptr<vkr::Buffer> stagingBuffer;
        std::unique_ptr<vkr::DeviceMemory> stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        stagingBufferMemory->copyFrom(m_vertices);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

        // TODO extract to some class
        copyBuffer(stagingBuffer->getHandle(), m_vertexBuffer->getHandle(), bufferSize);
    }

    void createIndexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

        std::unique_ptr<vkr::Buffer> stagingBuffer;
        std::unique_ptr<vkr::DeviceMemory> stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        stagingBufferMemory->copyFrom(m_indices);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

        // TODO extract to some class
        copyBuffer(stagingBuffer->getHandle(), m_indexBuffer->getHandle(), bufferSize);
    }

    void createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_uniformBuffers.resize(m_swapchain->getImageCount());
        m_uniformBuffersMemory.resize(m_swapchain->getImageCount());

        for (size_t i = 0; i < m_swapchain->getImageCount(); i++)
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);
    }

    void createDescriptorPool()
    {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(m_swapchain->getImageCount());
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(m_swapchain->getImageCount());

        VkDescriptorPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolCreateInfo.pPoolSizes = poolSizes.data();
        poolCreateInfo.maxSets = static_cast<uint32_t>(m_swapchain->getImageCount());

        if (vkCreateDescriptorPool(getDevice(), &poolCreateInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
            throw std::runtime_error("failed to create descriptor pool");
    }

    void createDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(m_swapchain->getImageCount(), m_descriptorSetLayout);

        VkDescriptorSetAllocateInfo descriptorSetallocInfo{};
        descriptorSetallocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetallocInfo.descriptorPool = m_descriptorPool;
        descriptorSetallocInfo.descriptorSetCount = static_cast<uint32_t>(m_swapchain->getImageCount());
        descriptorSetallocInfo.pSetLayouts = layouts.data();

        m_descriptorSets.resize(m_swapchain->getImageCount());
        if (vkAllocateDescriptorSets(getDevice(), &descriptorSetallocInfo, m_descriptorSets.data()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate descriptor sets!");

        for (size_t i = 0; i < m_swapchain->getImageCount(); i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_uniformBuffers[i]->getHandle();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = m_textureImageView->getHandle();
            imageInfo.sampler = m_textureSampler->getHandle();

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = m_descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = m_descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void createCommandBuffers()
    {
        m_commandBuffers.resize(m_swapchain->getImageCount());

        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = getRenderer()->getCommandPool();
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

        if (vkAllocateCommandBuffers(getDevice(), &commandBufferAllocateInfo, m_commandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate command buffers!");

        for (size_t i = 0; i < m_commandBuffers.size(); i++)
        {
            VkCommandBufferBeginInfo commandBufferBeginInfo{};
            commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            commandBufferBeginInfo.flags = 0;
            commandBufferBeginInfo.pInheritanceInfo = nullptr;

            if (vkBeginCommandBuffer(m_commandBuffers[i], &commandBufferBeginInfo) != VK_SUCCESS)
                throw std::runtime_error("failed to begin recording command buffer!");

            VkRenderPassBeginInfo renderPassBeginInfo{};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.renderPass = m_renderPass->getHandle();
            renderPassBeginInfo.framebuffer = m_swapchain->getFramebuffers()[i]->getHandle();
            renderPassBeginInfo.renderArea.offset = { 0, 0 };
            renderPassBeginInfo.renderArea.extent = m_swapchain->getExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
            clearValues[1].depthStencil = { 1.0f, 0 };

            renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());;
            renderPassBeginInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

            VkBuffer vertexBuffers[] = { m_vertexBuffer->getHandle() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(m_commandBuffers[i], m_indexBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[i], 0, nullptr);
            vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);

            vkCmdEndRenderPass(m_commandBuffers[i]);

            if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to record command buffer!");
        }
    }

    VkCommandBuffer beginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandPool = getRenderer()->getCommandPool();
        commandBufferAllocateInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(getDevice(), &commandBufferAllocateInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(getRenderer()->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(getRenderer()->getGraphicsQueue());

        vkFreeCommandBuffers(getDevice(), getRenderer()->getCommandPool(), 1, &commandBuffer);
    }

    void createSyncObjects()
    {
        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        m_imagesInFlight.resize(m_swapchain->getImageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(getDevice(), &semaphoreCreateInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to create image available semaphore!");

            if (vkCreateSemaphore(getDevice(), &semaphoreCreateInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to create render finished semaphore!");

            if (vkCreateFence(getDevice(), &fenceCreateInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to create in-flight fence!");
        }
    }

    void drawFrame()
    {
        vkWaitForFences(getDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult aquireImageResult = vkAcquireNextImageKHR(getDevice(), m_swapchain->getHandle(), UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (aquireImageResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapchain();
            return;
        }

        if (aquireImageResult != VK_SUCCESS && aquireImageResult != VK_SUBOPTIMAL_KHR)
            throw std::runtime_error("failed to acquire swapchain image!");

        if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
            vkWaitForFences(getDevice(), 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

        m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

        updateUniformBuffer(imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(getDevice(), 1, &m_inFlightFences[m_currentFrame]);

        if (vkQueueSubmit(getRenderer()->getGraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
            throw std::runtime_error("failed to submit draw command buffer!");

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapchains[] = { m_swapchain->getHandle() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        vkQueuePresentKHR(getRenderer()->getPresentQueue(), &presentInfo);

        if (aquireImageResult == VK_SUBOPTIMAL_KHR || m_framebufferResized)
        {
            m_framebufferResized = false;
            recreateSwapchain();
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void updateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        VkExtent2D swapchainExtent = m_swapchain->getExtent();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), 1.0f * swapchainExtent.width / swapchainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        m_uniformBuffersMemory[currentImage]->copyFrom(&ubo, sizeof(ubo));
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(m_window))
        {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(getDevice());
    }

    void cleanup()
    {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(getDevice(), m_renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(getDevice(), m_imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(getDevice(), m_inFlightFences[i], nullptr);
        }

        cleanupSwapchain();

        vkDestroyDescriptorSetLayout(getDevice(), m_descriptorSetLayout, nullptr);

        glfwDestroyWindow(m_window);

        glfwTerminate();
    }

    void cleanupSwapchain()
    {
        vkDestroyDescriptorPool(getDevice(), m_descriptorPool, nullptr);

        vkDestroyPipeline(getDevice(), m_pipeline, nullptr);
        vkDestroyPipelineLayout(getDevice(), m_pipelineLayout, nullptr);
    }

    std::unique_ptr<vkr::Renderer> const& getRenderer() const { return vkr::ServiceLocator::instance().getRenderer(); }
    VkDevice getDevice() const { return vkr::temp::getDevice(); }

private:
    GLFWwindow* m_window = nullptr;

    std::unique_ptr<vkr::Swapchain> m_swapchain;

    std::unique_ptr<vkr::RenderPass> m_renderPass;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;

    std::vector<VkCommandBuffer> m_commandBuffers;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;

    std::size_t m_currentFrame = 0;

    bool m_framebufferResized = false;

    std::unique_ptr<vkr::Buffer> m_vertexBuffer;
    std::unique_ptr<vkr::DeviceMemory> m_vertexBufferMemory;
    std::unique_ptr<vkr::Buffer> m_indexBuffer;
    std::unique_ptr<vkr::DeviceMemory> m_indexBufferMemory;

    std::vector<std::unique_ptr<vkr::Buffer>> m_uniformBuffers;
    std::vector<std::unique_ptr<vkr::DeviceMemory>> m_uniformBuffersMemory;

    VkDescriptorSetLayout m_descriptorSetLayout;
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;

    std::unique_ptr<vkr::Image> m_textureImage;
    std::unique_ptr<vkr::DeviceMemory> m_textureImageMemory;
    std::unique_ptr<vkr::ImageView> m_textureImageView;
    std::unique_ptr<vkr::Sampler> m_textureSampler;

    std::unique_ptr<vkr::Image> m_depthImage;
    std::unique_ptr<vkr::DeviceMemory> m_depthImageMemory;
    std::unique_ptr<vkr::ImageView> m_depthImageView;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
};

int main()
{
    try
    {
        HelloTriangleApplication app;
        app.run();
    }
    catch (const std::exception& e)
    {
        vkr::ServiceLocator::instance().setRenderer(nullptr);

        std::cerr << e.what() << std::endl;
        std::getchar();

        return EXIT_FAILURE;
    }

    // temporary to catch Vulkan errors
    vkr::ServiceLocator::instance().setRenderer(nullptr);
    std::getchar();
    return EXIT_SUCCESS;
}