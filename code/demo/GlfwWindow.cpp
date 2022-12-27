#include "GlfwWindow.h"

#include "vko/Surface.h"
#include "vko/Instance.h"

#include "nstl/vector.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace
{
    GlfwWindow* getWindowFromHandle(GLFWwindow* handle) noexcept
    {
        return static_cast<GlfwWindow*>(glfwGetWindowUserPointer(handle));
    }

    template<auto MemberFunc>
    auto createGlfwCallback()
    {
        return [](GLFWwindow* handle, auto... args) {
            auto window = ::getWindowFromHandle(handle);
            assert(window);
            (window->*MemberFunc)(nstl::forward<decltype(args)>(args)...);
        };
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

    auto translateCursorType(MouseCursorType type)
    {
        // TODO implement new cursors
        switch (type)
        {
            case MouseCursorType::Arrow:
                return GLFW_ARROW_CURSOR;
            case MouseCursorType::TextInput:
                return GLFW_IBEAM_CURSOR;
            case MouseCursorType::ResizeAll:
//                 return GLFW_RESIZE_ALL_CURSOR;
                return GLFW_ARROW_CURSOR;
            case MouseCursorType::ResizeNS:
                return GLFW_VRESIZE_CURSOR;
            case MouseCursorType::ResizeEW:
                return GLFW_HRESIZE_CURSOR;
            case MouseCursorType::ResizeNESW:
//                 return GLFW_RESIZE_NESW_CURSOR;
                return GLFW_ARROW_CURSOR;
            case MouseCursorType::ResizeNWSE:
//                 return GLFW_RESIZE_NWSE_CURSOR;
                return GLFW_ARROW_CURSOR;
            case MouseCursorType::NotAllowed:
//                 return GLFW_NOT_ALLOWED_CURSOR;
                return GLFW_ARROW_CURSOR;
            case MouseCursorType::Hand:
                return GLFW_HAND_CURSOR;
        }

        assert(false);
        return -1;
    }
}

GlfwWindow::GlfwWindow(int width, int height, char const* title)
{
    m_width = width;
    m_height = height;

    glfwInit();

    createWindow(title);
    setupCallbacks();
    queryRequiredInstanceExtensions();
    createCursors();
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

void GlfwWindow::setCursor(MouseCursorType type)
{
    glfwSetCursor(m_handle, m_cursors[static_cast<size_t>(type)]);
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

void GlfwWindow::setupCallbacks()
{
    glfwSetFramebufferSizeCallback(m_handle, createGlfwCallback<&GlfwWindow::onFramebufferResized>());
    glfwSetKeyCallback(m_handle, createGlfwCallback<&GlfwWindow::onKey>());
    glfwSetMouseButtonCallback(m_handle, createGlfwCallback<&GlfwWindow::onMouseButton>());
    glfwSetCursorPosCallback(m_handle, createGlfwCallback<&GlfwWindow::onCursorPosition>());
}

void GlfwWindow::queryRequiredInstanceExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    m_requiredInstanceExtensions = nstl::vector<char const*>(glfwExtensions, glfwExtensions + glfwExtensionCount); // TODO use std::span?
}

void GlfwWindow::createCursors()
{
    for (size_t i = 0; i < m_cursors.size(); i++)
    {
        auto glfwCursorType = translateCursorType(static_cast<MouseCursorType>(i));
        m_cursors[i] = glfwCreateStandardCursor(glfwCursorType);
        assert(m_cursors[i]);
    }
}

void GlfwWindow::onFramebufferResized(int width, int height)
{
    m_width = width;
    m_height = height;

    for (auto const& callback : m_resizeCallbacks)
        if (callback)
            callback(width, height);
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

void GlfwWindow::onMouseButton(int glfwButton, int glfwAction, int)
{
    if (m_canCaptureCursor && glfwButton == GLFW_MOUSE_BUTTON_LEFT)
    {
		Action action = getAction(glfwAction);

		m_cursorCaptured = action == Action::Press;
    }
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
