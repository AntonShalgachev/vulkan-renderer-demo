#pragma once

#include <string>
#include <vector>
#include <functional>

#include "glm.h"

struct GLFWwindow;

namespace vkr
{
    class Window
    {
        friend class VulkanSurfaceCreator;

    public:
        enum class Key
        {
            Unknown,
            Char,
        };

        enum class Modifiers
        {
            None = 0,
            Ctrl = 1,
            Shift = 2,
            Alt = 4,
        };

        enum class Action
        {
            Press,
            Release,
            Repeat,
        };

    public:
    	Window(int width, int height, std::string const& title);
    	~Window();

        Window(Window const&) = delete;
        Window(Window&&) = delete;
        Window& operator=(Window const&) = delete;
        Window& operator=(Window&&) = delete;

        GLFWwindow* getHandle() const { return m_handle; }

        void resize(int width, int height);

        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }

        std::vector<char const*> const& getRequiredInstanceExtensions() const { return m_requiredInstanceExtensions; }

        bool getCanCaptureCursor() const { return m_canCaptureCursor; }
        void setCanCaptureCursor(bool canCaptureCursor) { m_canCaptureCursor = canCaptureCursor; }

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

        template<typename Func>
        void addKeyCallback(Func&& callback)
        {
            m_keyCallbacks.emplace_back(std::forward<Func>(callback));
		}

		template<typename Func>
		void addMouseMoveCallback(Func&& callback)
		{
			m_mouseMoveCallbacks.emplace_back(std::forward<Func>(callback));
		}

        void waitUntilInForeground() const;

    private:
        void createWindow(std::string const& title);
        void queryRequiredInstanceExtensions();

        static void framebufferResizeCallback(GLFWwindow* window, int width, int height) noexcept;
        void onFramebufferResized(int width, int height);

        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept;
        void onKey(int key, int scancode, int action, int mods);

		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) noexcept;
        void onMouseButton(int button, int action, int mods);

		static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) noexcept;
        void onCursorPosition(double xpos, double ypos);

        bool shouldClose() const;
        void pollEvents() const;

    private:
        GLFWwindow* m_handle = nullptr;

        int m_width = -1;
        int m_height = -1;

        bool m_canCaptureCursor = true;
        bool m_cursorCaptured = false;
        glm::vec2 m_lastCursorPosition;

        std::vector<char const*> m_requiredInstanceExtensions;

        std::vector<std::function<void(int, int)>> m_resizeCallbacks;
        std::vector<std::function<void(Action, Key, char, Modifiers)>> m_keyCallbacks;
        std::vector<std::function<void(glm::vec2)>> m_mouseMoveCallbacks;
    };
}

inline vkr::Window::Modifiers operator|(vkr::Window::Modifiers lhs, vkr::Window::Modifiers rhs)
{
    using T = std::underlying_type_t<vkr::Window::Modifiers>;
    return static_cast<vkr::Window::Modifiers>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline vkr::Window::Modifiers& operator|=(vkr::Window::Modifiers& lhs, vkr::Window::Modifiers rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

inline auto operator&(vkr::Window::Modifiers lhs, vkr::Window::Modifiers rhs)
{
    using T = std::underlying_type_t<vkr::Window::Modifiers>;
    return static_cast<T>(lhs) & static_cast<T>(rhs);
}
