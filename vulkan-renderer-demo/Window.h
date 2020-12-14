#pragma once

#include <string>
#include <vector>
#include <functional>

struct GLFWwindow;

namespace vkr
{
    class Window
    {
        friend class VulkanSurfaceCreator;
    public:
    	Window(int width, int height, std::string const& title);
    	~Window();

        Window(Window const&) = delete;
        Window(Window&&) = default;
        Window& operator=(Window const&) = delete;
        Window& operator=(Window&&) = delete;

        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }

        template <typename Func>
        void startEventLoop(Func&& onUpdate)
        {
            while (!shouldClose())
            {
                pollEvents();
                onUpdate();
            }
        }

        template<typename Func>
        void addResizeCallback(Func&& callback)
        {
            m_resizeCallbacks.emplace_back(std::forward<Func>(callback));
        }

        void waitUntilInForeground() const;

    private:
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height) noexcept;
        void onFramebufferResized(int width, int height);

        bool shouldClose() const;
        void pollEvents() const;

    private:
        GLFWwindow* m_handle = nullptr;

        int m_width = -1;
        int m_height = -1;

        std::vector<std::function<void(int, int)>> m_resizeCallbacks;
    };
}
