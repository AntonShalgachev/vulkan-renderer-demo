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
	};
}