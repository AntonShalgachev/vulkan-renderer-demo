#include "Window.h"

#include <GLFW/glfw3.h>

namespace
{
    static vkr::Window* getAppFromWindow(GLFWwindow* window) noexcept
    {
        return reinterpret_cast<vkr::Window*>(glfwGetWindowUserPointer(window));
    }
}

vkr::Window::Window(int width, int height, std::string const& title)
{
    m_width = width;
    m_height = height;

    glfwInit();

    createWindow(title);
    glfwSetFramebufferSizeCallback(m_handle, Window::framebufferResizeCallback);
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

bool vkr::Window::shouldClose() const
{
    return glfwWindowShouldClose(m_handle);
}

void vkr::Window::pollEvents() const
{
    glfwPollEvents();
}