#pragma once

#include "vko/Window.h"

#include "nstl/vector.h"
#include "nstl/function.h"
#include "nstl/array.h"

struct GLFWwindow;
struct GLFWcursor;

enum class MouseCursorType
{
    Arrow = 0,
    TextInput,
    ResizeAll,
    ResizeNS,
    ResizeEW,
    ResizeNESW,
    ResizeNWSE,
    NotAllowed,
    Hand,

    COUNT,
};

class GlfwWindow : public vko::Window
{
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
    GlfwWindow(int width, int height, char const* title);
    ~GlfwWindow();

    GlfwWindow(GlfwWindow const&) = delete;
    GlfwWindow(GlfwWindow&&) = delete;
    GlfwWindow& operator=(GlfwWindow const&) = delete;
    GlfwWindow& operator=(GlfwWindow&&) = delete;

    GLFWwindow* getHandle() const { return m_handle; }

    void resize(int width, int height);

    vko::Surface createSurface(vko::Instance const& instance) const override;
    std::size_t getWidth() const override { return m_width; }
    std::size_t getHeight() const override { return m_height; }
    nstl::vector<char const*> const& getRequiredInstanceExtensions() const override { return m_requiredInstanceExtensions; }

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

    void addResizeCallback(nstl::function<void(int, int)> callback) override;
    void addKeyCallback(nstl::function<void(Action, Key, char, Modifiers)> callback);
    void addMouseMoveCallback(nstl::function<void(float deltaX, float deltaY)> callback);

    void waitUntilInForeground() const override;

    void setCursor(MouseCursorType type);

private:
    void createWindow(char const* title);
    void setupCallbacks();
    void queryRequiredInstanceExtensions();
    void createCursors();

    void onFramebufferResized(int width, int height);
    void onKey(int key, int scancode, int action, int mods);
    void onMouseButton(int button, int action, int mods);
    void onCursorPosition(double xpos, double ypos);

    bool shouldClose() const;
    void pollEvents() const;

private:
    GLFWwindow* m_handle = nullptr;

    int m_width = -1;
    int m_height = -1;

    bool m_canCaptureCursor = true;
    bool m_cursorCaptured = false;
    float m_lastCursorPositionX = 0.0f;
    float m_lastCursorPositionY = 0.0f;

    nstl::vector<char const*> m_requiredInstanceExtensions;

    nstl::vector<nstl::function<void(int, int)>> m_resizeCallbacks;
    nstl::vector<nstl::function<void(Action, Key, char, Modifiers)>> m_keyCallbacks;
    nstl::vector<nstl::function<void(float deltaX, float deltaY)>> m_mouseMoveCallbacks;

    nstl::array<GLFWcursor*, static_cast<size_t>(MouseCursorType::COUNT)> m_cursors;
};

inline GlfwWindow::Modifiers operator|(GlfwWindow::Modifiers lhs, GlfwWindow::Modifiers rhs)
{
    using T = std::underlying_type_t<GlfwWindow::Modifiers>;
    return static_cast<GlfwWindow::Modifiers>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline GlfwWindow::Modifiers& operator|=(GlfwWindow::Modifiers& lhs, GlfwWindow::Modifiers rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

inline auto operator&(GlfwWindow::Modifiers lhs, GlfwWindow::Modifiers rhs)
{
    using T = std::underlying_type_t<GlfwWindow::Modifiers>;
    return static_cast<T>(lhs) & static_cast<T>(rhs);
}
