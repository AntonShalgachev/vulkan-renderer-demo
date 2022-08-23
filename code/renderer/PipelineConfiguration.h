#pragma once

// TODO maybe remove?
#include "Shader.h"

#include "VertexLayoutDescriptions.h"

bool operator==(VkExtent2D const& lhs, VkExtent2D const& rhs);

namespace vko
{
    class PipelineLayout;
    class RenderPass;
}

namespace vkr
{
	struct PipelineConfiguration
	{
		vko::PipelineLayout const* pipelineLayout = nullptr;
		vko::RenderPass const* renderPass = nullptr;
		VkExtent2D extent{ 0, 0 }; // TODO don't use Vulkan struct here
		Shader::Key shaderKey;
		VertexLayoutDescriptions vertexLayoutDescriptions;
		bool cullBackFaces = true;
		bool wireframe = false;

		auto operator<=>(PipelineConfiguration const& rhs) const = default;
	};
}

namespace std
{
	template<>
	struct hash<vkr::PipelineConfiguration>
	{
		std::size_t operator()(vkr::PipelineConfiguration const& rhs) const
		{
			std::size_t seed = 0;
			vkr::hash::combine(seed, rhs.pipelineLayout);
			vkr::hash::combine(seed, rhs.renderPass);
			vkr::hash::combine(seed, rhs.extent);
			vkr::hash::combine(seed, rhs.shaderKey);
			vkr::hash::combine(seed, rhs.vertexLayoutDescriptions);
			vkr::hash::combine(seed, rhs.cullBackFaces);
			vkr::hash::combine(seed, rhs.wireframe);
			return seed;
		}
	};
}