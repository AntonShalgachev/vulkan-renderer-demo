#pragma once

// TODO maybe remove?
#include "Shader.h"

#include "VertexLayoutDescriptions.h"

namespace vkr
{
	class PipelineLayout;
	class RenderPass;

	struct PipelineConfiguration
	{
		PipelineLayout const* pipelineLayout;
		RenderPass const* renderPass;
		VkExtent2D extent;
		Shader::Key shaderKey;
		VertexLayoutDescriptions vertexLayoutDescriptions;

		bool operator==(PipelineConfiguration const& rhs) const;
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
			return seed;
		}
	};
}