#pragma once

#include <memory>

namespace vkr
{
	class Renderer;

	class ServiceLocator
	{
	public:
		static ServiceLocator& instance();

	public:
		const std::unique_ptr<Renderer>& getRenderer() const { return m_renderer; }
		void setRenderer(std::unique_ptr<Renderer> renderer) { m_renderer = std::move(renderer); }

	private:
		std::unique_ptr<Renderer> m_renderer;
	};
}
