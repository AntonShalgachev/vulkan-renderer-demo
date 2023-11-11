#include "platform_win64/window.h"

#include "nstl/string.h"
#include "nstl/enum.h"

#include <GLFW/glfw3.h>

namespace
{
    template<auto MemberFunc>
    auto createGlfwCallback()
    {
        return [](GLFWwindow* handle, auto... args) {
            platform_win64::glfw_window* window = static_cast<platform_win64::glfw_window*>(glfwGetWindowUserPointer(handle));
            assert(window);
            (window->*MemberFunc)(nstl::forward<decltype(args)>(args)...);
        };
    }

    auto get_glfw_cursor_type(platform::mouse_cursor_icon type)
    {
        // TODO implement new cursors
        switch (type)
        {
        case platform::mouse_cursor_icon::arrow:
            return GLFW_ARROW_CURSOR;
        case platform::mouse_cursor_icon::text_input:
            return GLFW_IBEAM_CURSOR;
        case platform::mouse_cursor_icon::resize_all:
            return GLFW_ARROW_CURSOR; // TODO use GLFW_RESIZE_ALL_CURSOR
        case platform::mouse_cursor_icon::resize_ns:
            return GLFW_VRESIZE_CURSOR;
        case platform::mouse_cursor_icon::resize_ew:
            return GLFW_HRESIZE_CURSOR;
        case platform::mouse_cursor_icon::resize_nesw:
            return GLFW_ARROW_CURSOR; // TODO use GLFW_RESIZE_NESW_CURSOR
        case platform::mouse_cursor_icon::resize_nwse:
            return GLFW_ARROW_CURSOR; // TODO use GLFW_RESIZE_NWSE_CURSOR
        case platform::mouse_cursor_icon::not_allowed:
            return GLFW_ARROW_CURSOR; // TODO use GLFW_NOT_ALLOWED_CURSOR
        case platform::mouse_cursor_icon::hand:
            return GLFW_HAND_CURSOR;
        }

        assert(false);
        return -1;
    }

    platform::button_action get_action(int action)
    {
        if (action == GLFW_PRESS)
            return platform::button_action::press;
        if (action == GLFW_RELEASE)
            return platform::button_action::release;
        if (action == GLFW_REPEAT)
            return platform::button_action::repeat;

        assert(false);
        return platform::button_action::press;
    }

    platform::button_modifiers get_modifiers(int mods)
    {
        platform::button_modifiers modifiers = platform::button_modifiers::none;

        if (mods & GLFW_MOD_CONTROL)
            modifiers = modifiers | platform::button_modifiers::ctrl;
        if (mods & GLFW_MOD_SHIFT)
            modifiers = modifiers | platform::button_modifiers::shift;
        if (mods & GLFW_MOD_ALT)
            modifiers = modifiers | platform::button_modifiers::alt;

        return modifiers;
    }

    platform::keyboard_button get_key(int key)
    {
        if (key == -1)
            return platform::keyboard_button::unknown;

        switch (key)
        {
        case GLFW_KEY_TAB: return platform::keyboard_button::tab;
        case GLFW_KEY_LEFT: return platform::keyboard_button::left_arrow;
        case GLFW_KEY_RIGHT: return platform::keyboard_button::right_arrow;
        case GLFW_KEY_UP: return platform::keyboard_button::up_arrow;
        case GLFW_KEY_DOWN: return platform::keyboard_button::down_arrow;
        case GLFW_KEY_PAGE_UP: return platform::keyboard_button::page_up;
        case GLFW_KEY_PAGE_DOWN: return platform::keyboard_button::page_down;
        case GLFW_KEY_HOME: return platform::keyboard_button::home;
        case GLFW_KEY_END: return platform::keyboard_button::end;
        case GLFW_KEY_INSERT: return platform::keyboard_button::insert;
        case GLFW_KEY_DELETE: return platform::keyboard_button::del;
        case GLFW_KEY_BACKSPACE: return platform::keyboard_button::backspace;
        case GLFW_KEY_SPACE: return platform::keyboard_button::space;
        case GLFW_KEY_ENTER: return platform::keyboard_button::enter;
        case GLFW_KEY_ESCAPE: return platform::keyboard_button::escape;
        case GLFW_KEY_APOSTROPHE: return platform::keyboard_button::apostrophe;
        case GLFW_KEY_COMMA: return platform::keyboard_button::comma;
        case GLFW_KEY_MINUS: return platform::keyboard_button::minus;
        case GLFW_KEY_PERIOD: return platform::keyboard_button::period;
        case GLFW_KEY_SLASH: return platform::keyboard_button::slash;
        case GLFW_KEY_SEMICOLON: return platform::keyboard_button::semicolon;
        case GLFW_KEY_EQUAL: return platform::keyboard_button::equal;
        case GLFW_KEY_LEFT_BRACKET: return platform::keyboard_button::left_bracket;
        case GLFW_KEY_BACKSLASH: return platform::keyboard_button::backslash;
        case GLFW_KEY_RIGHT_BRACKET: return platform::keyboard_button::right_bracket;
        case GLFW_KEY_GRAVE_ACCENT: return platform::keyboard_button::grave_accent;
        case GLFW_KEY_CAPS_LOCK: return platform::keyboard_button::caps_lock;
        case GLFW_KEY_SCROLL_LOCK: return platform::keyboard_button::scroll_lock;
        case GLFW_KEY_NUM_LOCK: return platform::keyboard_button::num_lock;
        case GLFW_KEY_PRINT_SCREEN: return platform::keyboard_button::print_screen;
        case GLFW_KEY_PAUSE: return platform::keyboard_button::pause;
        case GLFW_KEY_KP_0: return platform::keyboard_button::keypad0;
        case GLFW_KEY_KP_1: return platform::keyboard_button::keypad1;
        case GLFW_KEY_KP_2: return platform::keyboard_button::keypad2;
        case GLFW_KEY_KP_3: return platform::keyboard_button::keypad3;
        case GLFW_KEY_KP_4: return platform::keyboard_button::keypad4;
        case GLFW_KEY_KP_5: return platform::keyboard_button::keypad5;
        case GLFW_KEY_KP_6: return platform::keyboard_button::keypad6;
        case GLFW_KEY_KP_7: return platform::keyboard_button::keypad7;
        case GLFW_KEY_KP_8: return platform::keyboard_button::keypad8;
        case GLFW_KEY_KP_9: return platform::keyboard_button::keypad9;
        case GLFW_KEY_KP_DECIMAL: return platform::keyboard_button::keypad_decimal;
        case GLFW_KEY_KP_DIVIDE: return platform::keyboard_button::keypad_divide;
        case GLFW_KEY_KP_MULTIPLY: return platform::keyboard_button::keypad_multiply;
        case GLFW_KEY_KP_SUBTRACT: return platform::keyboard_button::keypad_subtract;
        case GLFW_KEY_KP_ADD: return platform::keyboard_button::keypad_add;
        case GLFW_KEY_KP_ENTER: return platform::keyboard_button::keypad_enter;
        case GLFW_KEY_KP_EQUAL: return platform::keyboard_button::keypad_equal;
        case GLFW_KEY_LEFT_SHIFT: return platform::keyboard_button::left_shift;
        case GLFW_KEY_LEFT_CONTROL: return platform::keyboard_button::left_ctrl;
        case GLFW_KEY_LEFT_ALT: return platform::keyboard_button::left_alt;
        case GLFW_KEY_LEFT_SUPER: return platform::keyboard_button::left_super;
        case GLFW_KEY_RIGHT_SHIFT: return platform::keyboard_button::right_shift;
        case GLFW_KEY_RIGHT_CONTROL: return platform::keyboard_button::right_ctrl;
        case GLFW_KEY_RIGHT_ALT: return platform::keyboard_button::right_alt;
        case GLFW_KEY_RIGHT_SUPER: return platform::keyboard_button::right_super;
        case GLFW_KEY_MENU: return platform::keyboard_button::menu;
        case GLFW_KEY_0: return platform::keyboard_button::zero;
        case GLFW_KEY_1: return platform::keyboard_button::one;
        case GLFW_KEY_2: return platform::keyboard_button::two;
        case GLFW_KEY_3: return platform::keyboard_button::three;
        case GLFW_KEY_4: return platform::keyboard_button::four;
        case GLFW_KEY_5: return platform::keyboard_button::five;
        case GLFW_KEY_6: return platform::keyboard_button::six;
        case GLFW_KEY_7: return platform::keyboard_button::seven;
        case GLFW_KEY_8: return platform::keyboard_button::eight;
        case GLFW_KEY_9: return platform::keyboard_button::nine;
        case GLFW_KEY_A: return platform::keyboard_button::a;
        case GLFW_KEY_B: return platform::keyboard_button::b;
        case GLFW_KEY_C: return platform::keyboard_button::c;
        case GLFW_KEY_D: return platform::keyboard_button::d;
        case GLFW_KEY_E: return platform::keyboard_button::e;
        case GLFW_KEY_F: return platform::keyboard_button::f;
        case GLFW_KEY_G: return platform::keyboard_button::g;
        case GLFW_KEY_H: return platform::keyboard_button::h;
        case GLFW_KEY_I: return platform::keyboard_button::i;
        case GLFW_KEY_J: return platform::keyboard_button::j;
        case GLFW_KEY_K: return platform::keyboard_button::k;
        case GLFW_KEY_L: return platform::keyboard_button::l;
        case GLFW_KEY_M: return platform::keyboard_button::m;
        case GLFW_KEY_N: return platform::keyboard_button::n;
        case GLFW_KEY_O: return platform::keyboard_button::o;
        case GLFW_KEY_P: return platform::keyboard_button::p;
        case GLFW_KEY_Q: return platform::keyboard_button::q;
        case GLFW_KEY_R: return platform::keyboard_button::r;
        case GLFW_KEY_S: return platform::keyboard_button::s;
        case GLFW_KEY_T: return platform::keyboard_button::t;
        case GLFW_KEY_U: return platform::keyboard_button::u;
        case GLFW_KEY_V: return platform::keyboard_button::v;
        case GLFW_KEY_W: return platform::keyboard_button::w;
        case GLFW_KEY_X: return platform::keyboard_button::x;
        case GLFW_KEY_Y: return platform::keyboard_button::y;
        case GLFW_KEY_Z: return platform::keyboard_button::z;
        case GLFW_KEY_F1: return platform::keyboard_button::f1;
        case GLFW_KEY_F2: return platform::keyboard_button::f2;
        case GLFW_KEY_F3: return platform::keyboard_button::f3;
        case GLFW_KEY_F4: return platform::keyboard_button::f4;
        case GLFW_KEY_F5: return platform::keyboard_button::f5;
        case GLFW_KEY_F6: return platform::keyboard_button::f6;
        case GLFW_KEY_F7: return platform::keyboard_button::f7;
        case GLFW_KEY_F8: return platform::keyboard_button::f8;
        case GLFW_KEY_F9: return platform::keyboard_button::f9;
        case GLFW_KEY_F10: return platform::keyboard_button::f10;
        case GLFW_KEY_F11: return platform::keyboard_button::f11;
        case GLFW_KEY_F12: return platform::keyboard_button::f12;
        }

        assert(false);
        return platform::keyboard_button::space;
    }

    platform::mouse_button get_mouse_button(int button)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            return platform::mouse_button::left;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            return platform::mouse_button::middle;
        case GLFW_MOUSE_BUTTON_RIGHT:
            return platform::mouse_button::right;
        }

        assert(false);
        return platform::mouse_button::left;
    }
}

platform_win64::glfw_window::glfw_window(size_t width, size_t height, char const* title)
{
    m_window_width = width;
    m_window_height = height;
    m_framebuffer_width = width;
    m_framebuffer_height = height;

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_handle = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title, nullptr, nullptr);
    glfwSetWindowUserPointer(m_handle, this);

    glfwSetWindowSizeCallback(m_handle, createGlfwCallback<&glfw_window::on_window_resized>());
    glfwSetFramebufferSizeCallback(m_handle, createGlfwCallback<&glfw_window::on_framebuffer_resized>());
    glfwSetWindowIconifyCallback(m_handle, createGlfwCallback<&glfw_window::on_iconified>());
    glfwSetKeyCallback(m_handle, createGlfwCallback<&glfw_window::on_keyboard_button>());
    glfwSetMouseButtonCallback(m_handle, createGlfwCallback<&glfw_window::on_mouse_button>());
    glfwSetCursorPosCallback(m_handle, createGlfwCallback<&glfw_window::on_cursor_position>());
    glfwSetWindowFocusCallback(m_handle, createGlfwCallback<&glfw_window::on_focus>());
    glfwSetCursorEnterCallback(m_handle, createGlfwCallback<&glfw_window::on_cursor_enter>());
    glfwSetScrollCallback(m_handle, createGlfwCallback<&glfw_window::on_scroll>());
    glfwSetCharCallback(m_handle, createGlfwCallback<&glfw_window::on_char>());

    for (platform::mouse_cursor_icon icon : tiny_ctti::enum_values<platform::mouse_cursor_icon>())
    {
        size_t index = static_cast<size_t>(icon);
        m_cursors[index] = glfwCreateStandardCursor(get_glfw_cursor_type(icon));
        assert(m_cursors[index]);
    }
}

platform_win64::glfw_window::~glfw_window()
{
    for (GLFWcursor*& cursor : m_cursors)
    {
        if (cursor)
            glfwDestroyCursor(cursor);
        cursor = nullptr;
    }

    glfwDestroyWindow(m_handle);
    glfwTerminate();
}

void platform_win64::glfw_window::poll_events()
{
    glfwPollEvents();
}

bool platform_win64::glfw_window::should_close() const
{
    return glfwWindowShouldClose(m_handle) > 0;
}

void platform_win64::glfw_window::resize(int width, int height)
{
    glfwSetWindowSize(m_handle, width, height);
}

void platform_win64::glfw_window::set_cursor_mode(platform::mouse_cursor_mode mode)
{
    auto covnert = [](platform::mouse_cursor_mode mode)
    {
        switch (mode)
        {
        case platform::mouse_cursor_mode::normal: return GLFW_CURSOR_NORMAL;
        case platform::mouse_cursor_mode::hidden: return GLFW_CURSOR_HIDDEN;
        case platform::mouse_cursor_mode::disabled: return GLFW_CURSOR_DISABLED;
        }

        assert(false);
        return GLFW_CURSOR_NORMAL;
    };

    glfwSetInputMode(m_handle, GLFW_CURSOR, covnert(mode));
    m_cursor_mode = mode;
}

void platform_win64::glfw_window::set_cursor_icon(platform::mouse_cursor_icon icon)
{
    glfwSetCursor(m_handle, m_cursors[static_cast<size_t>(icon)]);
    m_cursor_icon = icon;
}

char const* platform_win64::glfw_window::get_clipboard_text() const
{
    return glfwGetClipboardString(nullptr);
}

void platform_win64::glfw_window::set_clipboard_text(char const* text)
{
    glfwSetClipboardString(nullptr, text);
}

void platform_win64::glfw_window::on_window_resized(int width, int height)
{
    if (width < 0)
        width = 0;
    if (height < 0)
        height = 0;

    m_window_width = static_cast<size_t>(width);
    m_window_height = static_cast<size_t>(height);

    m_on_window_resize(m_window_width, m_window_height);
}

void platform_win64::glfw_window::on_framebuffer_resized(int width, int height)
{
    if (width < 0)
        width = 0;
    if (height < 0)
        height = 0;

    m_framebuffer_width = static_cast<size_t>(width);
    m_framebuffer_height = static_cast<size_t>(height);

    m_on_framebuffer_resize(m_framebuffer_width, m_framebuffer_height);
}

void platform_win64::glfw_window::on_iconified(int iconified)
{
    m_is_iconified = static_cast<bool>(iconified);
}

void platform_win64::glfw_window::on_keyboard_button(int key, int, int action, int mods)
{
    m_on_keyboard_button(get_action(action), get_key(key), get_modifiers(mods));
}

void platform_win64::glfw_window::on_mouse_button(int button, int action, int mods)
{
    m_on_mouse_button(get_action(action), get_mouse_button(button), get_modifiers(mods));
}

void platform_win64::glfw_window::on_cursor_position(double xpos, double ypos)
{
    float x = static_cast<float>(xpos);
    float y = static_cast<float>(ypos);

    m_on_mouse_position(x, y);

    float dx = x - m_last_mouse_pos_x;
    float dy = y - m_last_mouse_pos_y;

    m_last_mouse_pos_x = x;
    m_last_mouse_pos_y = y;

    m_on_mouse_delta(dx, dy);
}

void platform_win64::glfw_window::on_focus(int focused)
{
    m_on_focus(focused != 0);
}

void platform_win64::glfw_window::on_cursor_enter(int entered)
{
    m_on_cursor_enter(entered != 0);
}

void platform_win64::glfw_window::on_scroll(double xoffset, double yoffset)
{
    m_on_scroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void platform_win64::glfw_window::on_char(unsigned int c)
{
    assert(c <= CHAR_MAX);
    m_on_char(static_cast<char>(c));
}
