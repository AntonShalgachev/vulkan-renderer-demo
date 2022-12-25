#include "GlfwWindow.h"

#include "vko/Surface.h"
#include "vko/Instance.h"

#include "nstl/vector.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace
{
    static GlfwWindow* getAppFromWindow(GLFWwindow* window) noexcept
    {
        return static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    }

    GlfwWindow::Action getAction(int glfwAction)
    {
        static nstl::vector<GlfwWindow::Action> const actions = []() {
            nstl::vector<GlfwWindow::Action> result;
            result.resize(3);
            result[GLFW_RELEASE] = GlfwWindow::Action::Release;
            result[GLFW_PRESS] = GlfwWindow::Action::Press;
            result[GLFW_REPEAT] = GlfwWindow::Action::Repeat;
            return result;
        }();

        return actions[static_cast<std::size_t>(glfwAction)];
    }

    GlfwWindow::Modifiers getModifiers(int mods)
    {
        GlfwWindow::Modifiers modifiers = GlfwWindow::Modifiers::None;

        if (mods & GLFW_MOD_CONTROL)
            modifiers = modifiers | GlfwWindow::Modifiers::Ctrl;
        if (mods & GLFW_MOD_SHIFT)
            modifiers = modifiers | GlfwWindow::Modifiers::Shift;
        if (mods & GLFW_MOD_ALT)
            modifiers = modifiers | GlfwWindow::Modifiers::Alt;

        return modifiers;
    }
}

GlfwWindow::GlfwWindow(int width, int height, char const* title)
{
    m_width = width;
    m_height = height;

    glfwInit();

    createWindow(title);
    glfwSetFramebufferSizeCallback(m_handle, GlfwWindow::framebufferResizeCallback);
    glfwSetKeyCallback(m_handle, GlfwWindow::keyCallback);
    glfwSetMouseButtonCallback(m_handle, GlfwWindow::mouseButtonCallback);
    glfwSetCursorPosCallback(m_handle, GlfwWindow::cursorPositionCallback);
    queryRequiredInstanceExtensions();
}

void GlfwWindow::resize(int width, int height)
{
    glfwSetWindowSize(m_handle, width, height);
}

vko::Surface GlfwWindow::createSurface(vko::Instance const& instance) const
{
    VkSurfaceKHR handle;

    if (glfwCreateWindowSurface(instance.getHandle(), m_handle, nullptr, &handle) != VK_SUCCESS)
        assert(false);

    return vko::Surface{ handle, instance, *this };
}

void GlfwWindow::addResizeCallback(nstl::function<void(int, int)> callback)
{
    m_resizeCallbacks.emplace_back(nstl::move(callback));
}

void GlfwWindow::addKeyCallback(nstl::function<void(Action, Key, char, Modifiers)> callback)
{
    m_keyCallbacks.emplace_back(nstl::move(callback));
}

void GlfwWindow::addMouseMoveCallback(nstl::function<void(float deltaX, float deltaY)> callback)
{
    m_mouseMoveCallbacks.emplace_back(nstl::move(callback));
}

void GlfwWindow::waitUntilInForeground() const
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_handle, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_handle, &width, &height);
        glfwWaitEvents();
    }
}

GlfwWindow::~GlfwWindow()
{
    glfwDestroyWindow(m_handle);
    glfwTerminate();
}

void GlfwWindow::createWindow(char const* title)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_handle = glfwCreateWindow(m_width, m_height, title, nullptr, nullptr);
    glfwSetWindowUserPointer(m_handle, this);
}

void GlfwWindow::queryRequiredInstanceExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    m_requiredInstanceExtensions = nstl::vector<char const*>(glfwExtensions, glfwExtensions + glfwExtensionCount); // TODO use std::span?
}

void GlfwWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) noexcept
{
    if (auto app = ::getAppFromWindow(window))
        app->onFramebufferResized(width, height);
}

void GlfwWindow::onFramebufferResized(int width, int height)
{
    m_width = width;
    m_height = height;

    for (auto const& callback : m_resizeCallbacks)
    {
        if (callback)
            callback(width, height);
    }
}

void GlfwWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept
{
    if (auto app = ::getAppFromWindow(window))
        app->onKey(key, scancode, action, mods);
}

void GlfwWindow::onKey(int glfwKey, int, int glfwAction, int glfwMods)
{
    Action action = getAction(glfwAction);

    if (action == Action::Repeat)
        return;

    char c = '\0';
    Key key = Key::Unknown;
    if (glfwKey >= 0 && glfwKey < std::numeric_limits<char>::max())
    {
        key = Key::Char;
        c = static_cast<char>(glfwKey);
    }
    
    Modifiers mods = getModifiers(glfwMods);

    for (auto const& callback : m_keyCallbacks)
        if (callback)
            callback(action, key, c, mods);
}

void GlfwWindow::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) noexcept
{
	if (auto app = ::getAppFromWindow(window))
		app->onMouseButton(button, action, mods);
}

void GlfwWindow::onMouseButton(int glfwButton, int glfwAction, int)
{
    if (m_canCaptureCursor && glfwButton == GLFW_MOUSE_BUTTON_LEFT)
    {
		Action action = getAction(glfwAction);

		m_cursorCaptured = action == Action::Press;
    }
}

void GlfwWindow::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) noexcept
{
	if (auto app = ::getAppFromWindow(window))
		app->onCursorPosition(xpos, ypos);
}

void GlfwWindow::onCursorPosition(double xpos, double ypos)
{
    float deltaX = xpos - m_lastCursorPositionX;
    float deltaY = ypos - m_lastCursorPositionY;

    m_lastCursorPositionX = xpos;
    m_lastCursorPositionY = ypos;

    if (m_cursorCaptured)
    {
		for (auto const& callback : m_mouseMoveCallbacks)
			if (callback)
                callback(deltaX, deltaY);
    }
}

bool GlfwWindow::shouldClose() const
{
    return glfwWindowShouldClose(m_handle) > 0;
}

void GlfwWindow::pollEvents() const
{
    glfwPollEvents();
}
