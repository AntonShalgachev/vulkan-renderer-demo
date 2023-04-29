#include "GlfwWindow.h"

#include "vko/Surface.h"
#include "vko/Instance.h"

#include "nstl/vector.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <limits.h>

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
        if (glfwAction == GLFW_PRESS)
            return GlfwWindow::Action::Press;
        if (glfwAction == GLFW_RELEASE)
            return GlfwWindow::Action::Release;
        if (glfwAction == GLFW_REPEAT)
            return GlfwWindow::Action::Repeat;

        assert(false);
        return GlfwWindow::Action::Press;
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

    GlfwWindow::Key getKey(int key)
    {
        if (key == -1)
            return GlfwWindow::Key::Unknown;

        switch (key)
        {
        case GLFW_KEY_TAB: return GlfwWindow::Key::Tab;
        case GLFW_KEY_LEFT: return GlfwWindow::Key::LeftArrow;
        case GLFW_KEY_RIGHT: return GlfwWindow::Key::RightArrow;
        case GLFW_KEY_UP: return GlfwWindow::Key::UpArrow;
        case GLFW_KEY_DOWN: return GlfwWindow::Key::DownArrow;
        case GLFW_KEY_PAGE_UP: return GlfwWindow::Key::PageUp;
        case GLFW_KEY_PAGE_DOWN: return GlfwWindow::Key::PageDown;
        case GLFW_KEY_HOME: return GlfwWindow::Key::Home;
        case GLFW_KEY_END: return GlfwWindow::Key::End;
        case GLFW_KEY_INSERT: return GlfwWindow::Key::Insert;
        case GLFW_KEY_DELETE: return GlfwWindow::Key::Delete;
        case GLFW_KEY_BACKSPACE: return GlfwWindow::Key::Backspace;
        case GLFW_KEY_SPACE: return GlfwWindow::Key::Space;
        case GLFW_KEY_ENTER: return GlfwWindow::Key::Enter;
        case GLFW_KEY_ESCAPE: return GlfwWindow::Key::Escape;
        case GLFW_KEY_APOSTROPHE: return GlfwWindow::Key::Apostrophe;
        case GLFW_KEY_COMMA: return GlfwWindow::Key::Comma;
        case GLFW_KEY_MINUS: return GlfwWindow::Key::Minus;
        case GLFW_KEY_PERIOD: return GlfwWindow::Key::Period;
        case GLFW_KEY_SLASH: return GlfwWindow::Key::Slash;
        case GLFW_KEY_SEMICOLON: return GlfwWindow::Key::Semicolon;
        case GLFW_KEY_EQUAL: return GlfwWindow::Key::Equal;
        case GLFW_KEY_LEFT_BRACKET: return GlfwWindow::Key::LeftBracket;
        case GLFW_KEY_BACKSLASH: return GlfwWindow::Key::Backslash;
        case GLFW_KEY_RIGHT_BRACKET: return GlfwWindow::Key::RightBracket;
        case GLFW_KEY_GRAVE_ACCENT: return GlfwWindow::Key::GraveAccent;
        case GLFW_KEY_CAPS_LOCK: return GlfwWindow::Key::CapsLock;
        case GLFW_KEY_SCROLL_LOCK: return GlfwWindow::Key::ScrollLock;
        case GLFW_KEY_NUM_LOCK: return GlfwWindow::Key::NumLock;
        case GLFW_KEY_PRINT_SCREEN: return GlfwWindow::Key::PrintScreen;
        case GLFW_KEY_PAUSE: return GlfwWindow::Key::Pause;
        case GLFW_KEY_KP_0: return GlfwWindow::Key::Keypad0;
        case GLFW_KEY_KP_1: return GlfwWindow::Key::Keypad1;
        case GLFW_KEY_KP_2: return GlfwWindow::Key::Keypad2;
        case GLFW_KEY_KP_3: return GlfwWindow::Key::Keypad3;
        case GLFW_KEY_KP_4: return GlfwWindow::Key::Keypad4;
        case GLFW_KEY_KP_5: return GlfwWindow::Key::Keypad5;
        case GLFW_KEY_KP_6: return GlfwWindow::Key::Keypad6;
        case GLFW_KEY_KP_7: return GlfwWindow::Key::Keypad7;
        case GLFW_KEY_KP_8: return GlfwWindow::Key::Keypad8;
        case GLFW_KEY_KP_9: return GlfwWindow::Key::Keypad9;
        case GLFW_KEY_KP_DECIMAL: return GlfwWindow::Key::KeypadDecimal;
        case GLFW_KEY_KP_DIVIDE: return GlfwWindow::Key::KeypadDivide;
        case GLFW_KEY_KP_MULTIPLY: return GlfwWindow::Key::KeypadMultiply;
        case GLFW_KEY_KP_SUBTRACT: return GlfwWindow::Key::KeypadSubtract;
        case GLFW_KEY_KP_ADD: return GlfwWindow::Key::KeypadAdd;
        case GLFW_KEY_KP_ENTER: return GlfwWindow::Key::KeypadEnter;
        case GLFW_KEY_KP_EQUAL: return GlfwWindow::Key::KeypadEqual;
        case GLFW_KEY_LEFT_SHIFT: return GlfwWindow::Key::LeftShift;
        case GLFW_KEY_LEFT_CONTROL: return GlfwWindow::Key::LeftCtrl;
        case GLFW_KEY_LEFT_ALT: return GlfwWindow::Key::LeftAlt;
        case GLFW_KEY_LEFT_SUPER: return GlfwWindow::Key::LeftSuper;
        case GLFW_KEY_RIGHT_SHIFT: return GlfwWindow::Key::RightShift;
        case GLFW_KEY_RIGHT_CONTROL: return GlfwWindow::Key::RightCtrl;
        case GLFW_KEY_RIGHT_ALT: return GlfwWindow::Key::RightAlt;
        case GLFW_KEY_RIGHT_SUPER: return GlfwWindow::Key::RightSuper;
        case GLFW_KEY_MENU: return GlfwWindow::Key::Menu;
        case GLFW_KEY_0: return GlfwWindow::Key::Zero;
        case GLFW_KEY_1: return GlfwWindow::Key::One;
        case GLFW_KEY_2: return GlfwWindow::Key::Two;
        case GLFW_KEY_3: return GlfwWindow::Key::Three;
        case GLFW_KEY_4: return GlfwWindow::Key::Four;
        case GLFW_KEY_5: return GlfwWindow::Key::Five;
        case GLFW_KEY_6: return GlfwWindow::Key::Six;
        case GLFW_KEY_7: return GlfwWindow::Key::Seven;
        case GLFW_KEY_8: return GlfwWindow::Key::Eight;
        case GLFW_KEY_9: return GlfwWindow::Key::Nine;
        case GLFW_KEY_A: return GlfwWindow::Key::A;
        case GLFW_KEY_B: return GlfwWindow::Key::B;
        case GLFW_KEY_C: return GlfwWindow::Key::C;
        case GLFW_KEY_D: return GlfwWindow::Key::D;
        case GLFW_KEY_E: return GlfwWindow::Key::E;
        case GLFW_KEY_F: return GlfwWindow::Key::F;
        case GLFW_KEY_G: return GlfwWindow::Key::G;
        case GLFW_KEY_H: return GlfwWindow::Key::H;
        case GLFW_KEY_I: return GlfwWindow::Key::I;
        case GLFW_KEY_J: return GlfwWindow::Key::J;
        case GLFW_KEY_K: return GlfwWindow::Key::K;
        case GLFW_KEY_L: return GlfwWindow::Key::L;
        case GLFW_KEY_M: return GlfwWindow::Key::M;
        case GLFW_KEY_N: return GlfwWindow::Key::N;
        case GLFW_KEY_O: return GlfwWindow::Key::O;
        case GLFW_KEY_P: return GlfwWindow::Key::P;
        case GLFW_KEY_Q: return GlfwWindow::Key::Q;
        case GLFW_KEY_R: return GlfwWindow::Key::R;
        case GLFW_KEY_S: return GlfwWindow::Key::S;
        case GLFW_KEY_T: return GlfwWindow::Key::T;
        case GLFW_KEY_U: return GlfwWindow::Key::U;
        case GLFW_KEY_V: return GlfwWindow::Key::V;
        case GLFW_KEY_W: return GlfwWindow::Key::W;
        case GLFW_KEY_X: return GlfwWindow::Key::X;
        case GLFW_KEY_Y: return GlfwWindow::Key::Y;
        case GLFW_KEY_Z: return GlfwWindow::Key::Z;
        case GLFW_KEY_F1: return GlfwWindow::Key::F1;
        case GLFW_KEY_F2: return GlfwWindow::Key::F2;
        case GLFW_KEY_F3: return GlfwWindow::Key::F3;
        case GLFW_KEY_F4: return GlfwWindow::Key::F4;
        case GLFW_KEY_F5: return GlfwWindow::Key::F5;
        case GLFW_KEY_F6: return GlfwWindow::Key::F6;
        case GLFW_KEY_F7: return GlfwWindow::Key::F7;
        case GLFW_KEY_F8: return GlfwWindow::Key::F8;
        case GLFW_KEY_F9: return GlfwWindow::Key::F9;
        case GLFW_KEY_F10: return GlfwWindow::Key::F10;
        case GLFW_KEY_F11: return GlfwWindow::Key::F11;
        case GLFW_KEY_F12: return GlfwWindow::Key::F12;
        }

        assert(false);
        return GlfwWindow::Key::Space;
    }

    GlfwWindow::MouseButton getMouseButton(int button)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            return GlfwWindow::MouseButton::Left;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            return GlfwWindow::MouseButton::Middle;
        case GLFW_MOUSE_BUTTON_RIGHT:
            return GlfwWindow::MouseButton::Right;
        }

        assert(false);
        return GlfwWindow::MouseButton::Left;
    }

    auto translateCursorType(MouseCursorIcon type)
    {
        // TODO implement new cursors
        switch (type)
        {
            case MouseCursorIcon::Arrow:
                return GLFW_ARROW_CURSOR;
            case MouseCursorIcon::TextInput:
                return GLFW_IBEAM_CURSOR;
            case MouseCursorIcon::ResizeAll:
//                 return GLFW_RESIZE_ALL_CURSOR;
                return GLFW_ARROW_CURSOR;
            case MouseCursorIcon::ResizeNS:
                return GLFW_VRESIZE_CURSOR;
            case MouseCursorIcon::ResizeEW:
                return GLFW_HRESIZE_CURSOR;
            case MouseCursorIcon::ResizeNESW:
//                 return GLFW_RESIZE_NESW_CURSOR;
                return GLFW_ARROW_CURSOR;
            case MouseCursorIcon::ResizeNWSE:
//                 return GLFW_RESIZE_NWSE_CURSOR;
                return GLFW_ARROW_CURSOR;
            case MouseCursorIcon::NotAllowed:
//                 return GLFW_NOT_ALLOWED_CURSOR;
                return GLFW_ARROW_CURSOR;
            case MouseCursorIcon::Hand:
                return GLFW_HAND_CURSOR;
        }

        assert(false);
        return -1;
    }
}

GlfwWindow::GlfwWindow(int width, int height, char const* title)
{
    m_windowWidth = width;
    m_windowHeight = height;
    m_framebufferWidth = width;
    m_framebufferHeight = height;

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

void GlfwWindow::setCursorMode(MouseCursorMode mode)
{
    auto covnert = [](MouseCursorMode mode) {
        switch (mode)
        {
        case MouseCursorMode::Normal: return GLFW_CURSOR_NORMAL;
        case MouseCursorMode::Hidden: return GLFW_CURSOR_HIDDEN;
        case MouseCursorMode::Disabled: return GLFW_CURSOR_DISABLED;
        }

        assert(false);
        return GLFW_CURSOR_NORMAL;
    };

    glfwSetInputMode(m_handle, GLFW_CURSOR, covnert(mode));
    m_cursorMode = mode;
}

MouseCursorMode GlfwWindow::getCursorMode() const
{
    return m_cursorMode;
}

void GlfwWindow::setCursorIcon(MouseCursorIcon icon)
{
    glfwSetCursor(m_handle, m_cursors[static_cast<size_t>(icon)]);
    m_cursorIcon = icon;
}

MouseCursorIcon GlfwWindow::getCursorIcon() const
{
    return m_cursorIcon;
}

char const* GlfwWindow::getClipboardText() const
{
    return glfwGetClipboardString(m_handle);
}

void GlfwWindow::setClipboardText(char const* text)
{
    glfwSetClipboardString(m_handle, text);
}

GlfwWindow::~GlfwWindow()
{
    destroyCursors();

    glfwDestroyWindow(m_handle);
    glfwTerminate();
}

void GlfwWindow::createWindow(char const* title)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_handle = glfwCreateWindow(m_windowWidth, m_windowHeight, title, nullptr, nullptr);
    glfwSetWindowUserPointer(m_handle, this);
}

void GlfwWindow::setupCallbacks()
{
    glfwSetWindowSizeCallback(m_handle, createGlfwCallback<&GlfwWindow::onWindowResized>());
    glfwSetFramebufferSizeCallback(m_handle, createGlfwCallback<&GlfwWindow::onFramebufferResized>());
    glfwSetKeyCallback(m_handle, createGlfwCallback<&GlfwWindow::onKey>());
    glfwSetMouseButtonCallback(m_handle, createGlfwCallback<&GlfwWindow::onMouseButton>());
    glfwSetCursorPosCallback(m_handle, createGlfwCallback<&GlfwWindow::onCursorPosition>());
    glfwSetWindowFocusCallback(m_handle, createGlfwCallback<&GlfwWindow::onFocus>());
    glfwSetCursorEnterCallback(m_handle, createGlfwCallback<&GlfwWindow::onCursorEnter>());
    glfwSetScrollCallback(m_handle, createGlfwCallback<&GlfwWindow::onScroll>());
    glfwSetCharCallback(m_handle, createGlfwCallback<&GlfwWindow::onChar>());
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
    for (MouseCursorIcon icon : tiny_ctti::enum_values<MouseCursorIcon>())
    {
        size_t index = static_cast<size_t>(icon);
        m_cursors[index] = glfwCreateStandardCursor(translateCursorType(icon));
        assert(m_cursors[index]);
    }
}

void GlfwWindow::destroyCursors()
{
    for (GLFWcursor*& cursor : m_cursors)
    {
        if (cursor)
            glfwDestroyCursor(cursor);
        cursor = nullptr;
    }
}

void GlfwWindow::onWindowResized(int width, int height)
{
    m_windowWidth = width;
    m_windowHeight = height;

    m_onWindowResize(width, height);
}

void GlfwWindow::onFramebufferResized(int width, int height)
{
    m_framebufferWidth = width;
    m_framebufferHeight = height;

    m_onFramebufferResize(width, height);
}

void GlfwWindow::onKey(int glfwKey, int gltfScancode, int glfwAction, int glfwMods)
{
    m_onKey(getAction(glfwAction), getKey(glfwKey), getModifiers(glfwMods));

    char c = '\0';
    OldKey key = OldKey::Unknown;
    if (glfwKey >= 0 && glfwKey < CHAR_MAX)
    {
        key = OldKey::Char;
        c = static_cast<char>(glfwKey);
    }
    
    m_onKeyOld(getAction(glfwAction), key, c, getModifiers(glfwMods));
}

void GlfwWindow::onMouseButton(int glfwButton, int glfwAction, int glfwMods)
{
    Action action = getAction(glfwAction);
    Modifiers mods = getModifiers(glfwMods);
    MouseButton button = getMouseButton(glfwButton);

    m_onMouseButton(action, button, mods);

    if (m_canCaptureCursor && glfwButton == GLFW_MOUSE_BUTTON_LEFT)
		m_cursorCaptured = action == Action::Press;
}

void GlfwWindow::onCursorPosition(double xpos, double ypos)
{
    m_onCursorPosition(static_cast<float>(xpos), static_cast<float>(ypos));

    float deltaX = xpos - m_lastCursorPositionX;
    float deltaY = ypos - m_lastCursorPositionY;

    m_lastCursorPositionX = xpos;
    m_lastCursorPositionY = ypos;

    if (m_cursorCaptured)
        m_onMouseDeltaOld(deltaX, deltaY);
}

void GlfwWindow::onFocus(int focused)
{
    m_onFocus(focused != 0);
}

void GlfwWindow::onCursorEnter(int entered)
{
    m_onCursorEnter(entered != 0);
}

void GlfwWindow::onScroll(double xoffset, double yoffset)
{
    m_onScroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void GlfwWindow::onChar(unsigned int c)
{
    m_onChar(c);
}

bool GlfwWindow::shouldClose() const
{
    return glfwWindowShouldClose(m_handle) > 0;
}

void GlfwWindow::pollEvents() const
{
    glfwPollEvents();
}
