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
        Window(Window&&) = delete;
        Window& operator=(Window const&) = delete;
        Window& operator=(Window&&) = delete;

        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }

        std::vector<char const*> const& getRequiredInstanceExtensions() const { return m_requiredInstanceExtensions; }

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
        void createWindow(std::string const& title);
        void queryRequiredInstanceExtensions();

        static void framebufferResizeCallback(GLFWwindow* window, int width, int height) noexcept;
        void onFramebufferResized(int width, int height);

        bool shouldClose() const;
        void pollEvents() const;

    private:
        GLFWwindow* m_handle = nullptr;

        int m_width = -1;
        int m_height = -1;

        std::vector<char const*> m_requiredInstanceExtensions;

        std::vector<std::function<void(int, int)>> m_resizeCallbacks;
    };
}
