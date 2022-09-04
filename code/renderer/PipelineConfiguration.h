#pragma once

// TODO maybe remove?
#include "Shader.h"

#include "VertexLayoutDescriptions.h"

namespace vko
{
    class PipelineLayout;
}

namespace vkr
{
	// TODO remove this struct and merge with vko::Pipeline::Config
	struct PipelineConfiguration
	{
		vko::PipelineLayout const* pipelineLayout = nullptr;
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
			vkr::hash::combine(seed, rhs.shaderKey);
			vkr::hash::combine(seed, rhs.vertexLayoutDescriptions);
			vkr::hash::combine(seed, rhs.cullBackFaces);
			vkr::hash::combine(seed, rhs.wireframe);
			return seed;
		}
	};
}