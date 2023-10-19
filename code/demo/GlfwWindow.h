#pragma once

#include "common/tiny_ctti.h"

#include "platform/window.h"

#include "tglm/types/ivec2.h"

#include "nstl/vector.h"
#include "nstl/function.h"
#include "nstl/array.h"

// TODO move to nstl
namespace temp
{
    template<typename... Args>
    class event
    {
    public:
        void add(nstl::function<void(Args...)> delegate)
        {
            m_delegates.push_back(nstl::move(delegate));
        }

        void operator()(Args const&... args)
        {
            for (auto const& delegate : m_delegates)
                if (delegate)
                    delegate(args...);
        }

    private:
        nstl::vector<nstl::function<void(Args...)>> m_delegates;
    };
}

struct GLFWwindow;
struct GLFWcursor;

// TODO unify location with GlfwWindow enums
enum class MouseCursorMode
{
    Normal,
    Hidden,
    Disabled,
};
TINY_CTTI_DESCRIBE_ENUM(MouseCursorMode, Normal, Hidden, Disabled);

enum class MouseCursorIcon
{
    Arrow,
    TextInput,
    ResizeAll,
    ResizeNS,
    ResizeEW,
    ResizeNESW,
    ResizeNWSE,
    NotAllowed,
    Hand,
};
TINY_CTTI_DESCRIBE_ENUM(MouseCursorIcon, Arrow, TextInput, ResizeAll, ResizeNS, ResizeEW, ResizeNESW, ResizeNWSE, NotAllowed, Hand);

class GlfwWindow final
{
public:
    // TODO remove this enum
    enum class OldKey
    {
        Unknown,
        Char,
    };
    TINY_CTTI_DESCRIBE_NESTED_ENUM(OldKey, Unknown, Char);

    // TODO compat some entries
    enum class Key
    {
        Unknown,
        Tab,
        LeftArrow, RightArrow, UpArrow, DownArrow,
        PageUp, PageDown, Home, End, Insert, Delete,
        Backspace,
        Space,
        Enter,
        Escape,
        Apostrophe,
        Comma,
        Minus,
        Period,
        Slash,
        Semicolon,
        Equal,
        LeftBracket,
        Backslash,
        RightBracket,
        GraveAccent,
        CapsLock,
        ScrollLock,
        NumLock,
        PrintScreen,
        Pause,
        Keypad0, Keypad1, Keypad2, Keypad3, Keypad4, Keypad5, Keypad6, Keypad7, Keypad8, Keypad9,
        KeypadDecimal, KeypadDivide, KeypadMultiply, KeypadSubtract, KeypadAdd, KeypadEnter, KeypadEqual,
        LeftShift, LeftCtrl, LeftAlt, LeftSuper, RightShift, RightCtrl, RightAlt, RightSuper,
        Menu,
        Zero, One, Two, Three, Four, Five, Six, Seven, Eight, Nine,
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    };

    enum class MouseButton
    {
        Left,
        Middle,
        Right,
    };
    TINY_CTTI_DESCRIBE_NESTED_ENUM(MouseButton, Left, Middle, Right);

    enum class Modifiers
    {
        None = 0,
        Ctrl = 1,
        Shift = 2,
        Alt = 4,
    };
    TINY_CTTI_DESCRIBE_NESTED_ENUM(Modifiers, None, Ctrl, Shift, Alt);

    enum class Action
    {
        Press,
        Release,
        Repeat,
    };
    TINY_CTTI_DESCRIBE_NESTED_ENUM(Action, Press, Release, Repeat);

public:
    GlfwWindow(int width, int height, char const* title);
    ~GlfwWindow();

    GlfwWindow(GlfwWindow const&) = delete;
    GlfwWindow(GlfwWindow&&) = delete;
    GlfwWindow& operator=(GlfwWindow const&) = delete;
    GlfwWindow& operator=(GlfwWindow&&) = delete;

    GLFWwindow* getHandle() const { return m_handle; }

    void resize(int width, int height);

    platform::window_handle_t getWindowHandle() const;
    size_t getWindowWidth() const { assert(m_windowWidth >= 0); return static_cast<size_t>(m_windowWidth); }
    size_t getWindowHeight() const { assert(m_windowHeight >= 0); return static_cast<size_t>(m_windowHeight); }
    size_t getFramebufferWidth() const { assert(m_framebufferWidth >= 0); return static_cast<size_t>(m_framebufferWidth); }
    size_t getFramebufferHeight() const { assert(m_framebufferHeight >= 0); return static_cast<size_t>(m_framebufferHeight); }
    tglm::ivec2 getFramebufferSize() const { return { m_framebufferWidth, m_framebufferHeight }; }
    bool isIconified() const { return m_iconified; }
    nstl::vector<char const*> const& getRequiredInstanceExtensions() const { return m_requiredInstanceExtensions; }

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

    void addWindowResizeCallback(nstl::function<void(int, int)> callback) { m_onWindowResize.add(nstl::move(callback)); }
    void addFramebufferResizeCallback(nstl::function<void(int, int)> callback) { m_onFramebufferResize.add(nstl::move(callback)); }
    void addKeyCallback(nstl::function<void(Action, Key, Modifiers)> callback) { m_onKey.add(nstl::move(callback)); }
    void addOldKeyCallback(nstl::function<void(Action, OldKey, char, Modifiers)> callback) { m_onKeyOld.add(nstl::move(callback)); }
    void addMouseButtonCallback(nstl::function<void(Action, MouseButton, Modifiers)> callback) { m_onMouseButton.add(nstl::move(callback)); }
    void addCursorPositionCallback(nstl::function<void(float, float)> callback) { m_onCursorPosition.add(nstl::move(callback)); }
    void addOldMouseDeltaCallback(nstl::function<void(float deltaX, float deltaY)> callback) { m_onMouseDeltaOld.add(nstl::move(callback)); }
    void addFocusCallback(nstl::function<void(bool)> callback) { m_onFocus.add(nstl::move(callback)); }
    void addCursorEnterCallback(nstl::function<void(bool)> callback) { m_onCursorEnter.add(nstl::move(callback)); }
    void addScrollCallback(nstl::function<void(float, float)> callback) { m_onScroll.add(nstl::move(callback)); }
    void addCharCallback(nstl::function<void(char)> callback) { m_onChar.add(nstl::move(callback)); }

    void waitUntilInForeground() const;

    void setCursorMode(MouseCursorMode mode);
    MouseCursorMode getCursorMode() const;

    void setCursorIcon(MouseCursorIcon icon);
    MouseCursorIcon getCursorIcon() const;

    char const* getClipboardText() const;
    void setClipboardText(char const* text);

private:
    void createWindow(char const* title);
    void setupCallbacks();
    void queryRequiredInstanceExtensions();
    void createCursors();

    void destroyCursors();
    
    void onWindowResized(int width, int height);
    void onFramebufferResized(int width, int height);
    void onIconified(int iconified);
    void onKey(int key, int scancode, int action, int mods);
    void onMouseButton(int button, int action, int mods);
    void onCursorPosition(double xpos, double ypos);
    void onFocus(int focused);
    void onCursorEnter(int entered);
    void onScroll(double xoffset, double yoffset);
    void onChar(unsigned int c);

    bool shouldClose() const;
    void pollEvents() const;

private:
    GLFWwindow* m_handle = nullptr;

    int m_windowWidth = -1;
    int m_windowHeight = -1;
    int m_framebufferWidth = -1;
    int m_framebufferHeight = -1;

    bool m_canCaptureCursor = true; // TODO remove
    bool m_cursorCaptured = false; // TODO remove
    float m_lastCursorPositionX = 0.0f;
    float m_lastCursorPositionY = 0.0f;

    MouseCursorMode m_cursorMode = MouseCursorMode::Normal;
    MouseCursorIcon m_cursorIcon = MouseCursorIcon::Arrow;

    bool m_iconified = false;

    nstl::vector<char const*> m_requiredInstanceExtensions;

    temp::event<int, int> m_onWindowResize;
    temp::event<int, int> m_onFramebufferResize;
    temp::event<Action, Key, Modifiers> m_onKey;
    temp::event<Action, OldKey, char, Modifiers> m_onKeyOld; // TODO remove
    temp::event<Action, MouseButton, Modifiers> m_onMouseButton;
    temp::event<float, float> m_onCursorPosition;
    temp::event<float, float> m_onMouseDeltaOld; // TODO remove
    temp::event<bool> m_onFocus;
    temp::event<bool> m_onCursorEnter;
    temp::event<float, float> m_onScroll;
    temp::event<char> m_onChar;

    nstl::array<GLFWcursor*, tiny_ctti::enum_size_v<MouseCursorIcon>> m_cursors;
};

inline GlfwWindow::Modifiers operator|(GlfwWindow::Modifiers lhs, GlfwWindow::Modifiers rhs)
{
    using T = nstl::underlying_type_t<GlfwWindow::Modifiers>;
    return static_cast<GlfwWindow::Modifiers>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline GlfwWindow::Modifiers& operator|=(GlfwWindow::Modifiers& lhs, GlfwWindow::Modifiers rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

inline auto operator&(GlfwWindow::Modifiers lhs, GlfwWindow::Modifiers rhs)
{
    using T = nstl::underlying_type_t<GlfwWindow::Modifiers>;
    return static_cast<T>(lhs) & static_cast<T>(rhs);
}
