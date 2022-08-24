#include "GlfwWindow.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace
{
    static vkr::GlfwWindow* getAppFromWindow(GLFWwindow* window) noexcept
    {
        return static_cast<vkr::GlfwWindow*>(glfwGetWindowUserPointer(window));
    }

    vkr::GlfwWindow::Action getAction(int glfwAction)
    {
        static std::vector<vkr::GlfwWindow::Action> const actions = []() {
            std::vector<vkr::GlfwWindow::Action> result;
            result.resize(3);
            result[GLFW_RELEASE] = vkr::GlfwWindow::Action::Release;
            result[GLFW_PRESS] = vkr::GlfwWindow::Action::Press;
            result[GLFW_REPEAT] = vkr::GlfwWindow::Action::Repeat;
            return result;
        }();

        return actions[static_cast<std::size_t>(glfwAction)];
    }

    vkr::GlfwWindow::Modifiers getModifiers(int mods)
    {
        vkr::GlfwWindow::Modifiers modifiers = vkr::GlfwWindow::Modifiers::None;

        if (mods & GLFW_MOD_CONTROL)
            modifiers = modifiers | vkr::GlfwWindow::Modifiers::Ctrl;
        if (mods & GLFW_MOD_SHIFT)
            modifiers = modifiers | vkr::GlfwWindow::Modifiers::Shift;
        if (mods & GLFW_MOD_ALT)
            modifiers = modifiers | vkr::GlfwWindow::Modifiers::Alt;

        return modifiers;
    }
}

vkr::GlfwWindow::GlfwWindow(int width, int height, std::string const& title)
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

void vkr::GlfwWindow::resize(int width, int height)
{
    glfwSetWindowSize(m_handle, width, height);
}

void vkr::GlfwWindow::waitUntilInForeground() const
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_handle, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_handle, &width, &height);
        glfwWaitEvents();
    }
}

vkr::GlfwWindow::~GlfwWindow()
{
    glfwDestroyWindow(m_handle);
    glfwTerminate();
}

void vkr::GlfwWindow::createWindow(std::string const& title)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_handle = glfwCreateWindow(m_width, m_height, title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(m_handle, this);
}

void vkr::GlfwWindow::queryRequiredInstanceExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    m_requiredInstanceExtensions.assign(glfwExtensions, glfwExtensions + glfwExtensionCount);
}

void vkr::GlfwWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) noexcept
{
    if (auto app = ::getAppFromWindow(window))
        app->onFramebufferResized(width, height);
}

void vkr::GlfwWindow::onFramebufferResized(int width, int height)
{
    m_width = width;
    m_height = height;

    for (auto const& callback : m_resizeCallbacks)
    {
        if (callback)
            callback(width, height);
    }
}

void vkr::GlfwWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept
{
    if (auto app = ::getAppFromWindow(window))
        app->onKey(key, scancode, action, mods);
}

void vkr::GlfwWindow::onKey(int glfwKey, int, int glfwAction, int glfwMods)
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

void vkr::GlfwWindow::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) noexcept
{
	if (auto app = ::getAppFromWindow(window))
		app->onMouseButton(button, action, mods);
}

void vkr::GlfwWindow::onMouseButton(int glfwButton, int glfwAction, int)
{
    if (m_canCaptureCursor && glfwButton == GLFW_MOUSE_BUTTON_LEFT)
    {
		Action action = getAction(glfwAction);

		m_cursorCaptured = action == Action::Press;
    }
}

void vkr::GlfwWindow::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) noexcept
{
	if (auto app = ::getAppFromWindow(window))
		app->onCursorPosition(xpos, ypos);
}

void vkr::GlfwWindow::onCursorPosition(double xpos, double ypos)
{
    glm::vec2 pos = { static_cast<float>(xpos), static_cast<float>(ypos) };
	glm::vec2 delta = pos - m_lastCursorPosition;
	m_lastCursorPosition = pos;

    if (m_cursorCaptured)
    {
		for (auto const& callback : m_mouseMoveCallbacks)
			if (callback)
				callback(delta);
    }
}

bool vkr::GlfwWindow::shouldClose() const
{
    return glfwWindowShouldClose(m_handle) > 0;
}

void vkr::GlfwWindow::pollEvents() const
{
    glfwPollEvents();
}
