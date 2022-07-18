#pragma once

#include "Transform.h"

namespace vkr
{
	class Drawable;
	class Camera;

	class SceneObject
	{
	public:
		Transform& getTransform() { return m_transform; }
		Transform const& getTransform() const { return m_transform; }

		void setDrawable(std::shared_ptr<Drawable> drawable) { m_drawable = std::move(drawable); }
		Drawable* getDrawable() const { return m_drawable.get(); }

		void setCamera(std::shared_ptr<Camera> camera) { m_camera = std::move(camera); }
		Camera* getCamera() const { return m_camera.get(); }

	private:
		Transform m_transform;

		std::shared_ptr<Drawable> m_drawable;
		std::shared_ptr<Camera> m_camera;
	};
}
