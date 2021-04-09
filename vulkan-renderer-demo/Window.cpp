#include "Window.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace
{
    static vkr::Window* getAppFromWindow(GLFWwindow* window) noexcept
    {
        return reinterpret_cast<vkr::Window*>(glfwGetWindowUserPointer(window));
    }

    vkr::Window::Action getAction(int glfwAction)
    {
        static std::vector<vkr::Window::Action> const actions = []() {
            std::vector<vkr::Window::Action> result;
            result.resize(3);
            result[GLFW_RELEASE] = vkr::Window::Action::Release;
            result[GLFW_PRESS] = vkr::Window::Action::Press;
            result[GLFW_REPEAT] = vkr::Window::Action::Repeat;
            return result;
        }();

        return actions[static_cast<std::size_t>(glfwAction)];
    }

    vkr::Window::Modifiers getModifiers(int mods)
    {
        vkr::Window::Modifiers modifiers = vkr::Window::Modifiers::None;

        if (mods & GLFW_MOD_CONTROL)
            modifiers = modifiers | vkr::Window::Modifiers::Ctrl;
        if (mods & GLFW_MOD_SHIFT)
            modifiers = modifiers | vkr::Window::Modifiers::Shift;
        if (mods & GLFW_MOD_ALT)
            modifiers = modifiers | vkr::Window::Modifiers::Alt;

        return modifiers;
    }
}

vkr::Window::Window(int width, int height, std::string const& title)
{
    m_width = width;
    m_height = height;

    glfwInit();

    createWindow(title);
    glfwSetFramebufferSizeCallback(m_handle, Window::framebufferResizeCallback);
    glfwSetKeyCallback(m_handle, Window::keyCallback);
    queryRequiredInstanceExtensions();
}

void vkr::Window::waitUntilInForeground() const
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_handle, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_handle, &width, &height);
        glfwWaitEvents();
    }
}

vkr::Window::~Window()
{
    glfwDestroyWindow(m_handle);
    glfwTerminate();
}

void vkr::Window::createWindow(std::string const& title)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_handle = glfwCreateWindow(m_width, m_height, title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(m_handle, this);
}

void vkr::Window::queryRequiredInstanceExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    m_requiredInstanceExtensions.assign(glfwExtensions, glfwExtensions + glfwExtensionCount);
}

void vkr::Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) noexcept
{
    if (auto app = ::getAppFromWindow(window))
        app->onFramebufferResized(width, height);
}

void vkr::Window::onFramebufferResized(int width, int height)
{
    m_width = width;
    m_height = height;

    for (auto const& callback : m_resizeCallbacks)
    {
        if (callback)
            callback(width, height);
    }
}

void vkr::Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept
{
    if (auto app = ::getAppFromWindow(window))
        app->onKey(key, scancode, action, mods);
}

void vkr::Window::onKey(int glfwKey, int, int glfwAction, int glfwMods)
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

bool vkr::Window::shouldClose() const
{
    return glfwWindowShouldClose(m_handle) > 0;
}

void vkr::Window::pollEvents() const
{
    glfwPollEvents();
}
