#include "PipelineConfiguration.h"

bool vkr::PipelineConfiguration::operator==(PipelineConfiguration const& rhs) const
{
	auto tie = [](PipelineConfiguration const& obj)
	{
		// TODO add other fields
		return std::tie(obj.pipelineLayout, obj.renderPass/*, obj.extent, obj.shaderKey, obj.vertexLayoutDescriptions*/);
	};

	return tie(*this) == tie(rhs);
}
