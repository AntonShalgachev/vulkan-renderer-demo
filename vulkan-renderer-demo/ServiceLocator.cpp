#include "ServiceLocator.h"

#include "Renderer.h"

namespace vkr
{
	vkr::ServiceLocator& ServiceLocator::instance()
	{
		static ServiceLocator instance;
		return instance;
	}
}
