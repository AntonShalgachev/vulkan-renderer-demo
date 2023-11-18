#include "platform_win64/window.h"

#include "common.h"

#include "nstl/string.h"
#include "nstl/buffer.h"

#include <Windowsx.h>

#include <GLFW/glfw3.h>

#include <limits.h>

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

    nstl::optional<platform::keyboard_button> get_key(int key)
    {
        if (key == -1)
            return {};

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

bool platform_win64::glfw_window::is_closed() const
{
    return glfwWindowShouldClose(m_handle) > 0;
}

void platform_win64::glfw_window::resize(size_t width, size_t height)
{
    assert(width <= INT_MAX);
    assert(height <= INT_MAX);
    glfwSetWindowSize(m_handle, static_cast<int>(width), static_cast<int>(height));
}

void platform_win64::glfw_window::set_cursor_mode(platform::mouse_cursor_mode mode)
{
    if (m_cursor_mode == mode)
        return;

    auto covnert = [](platform::mouse_cursor_mode mode)
    {
        switch (mode)
        {
        case platform::mouse_cursor_mode::visible: return GLFW_CURSOR_NORMAL;
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
    if (m_cursor_icon == icon)
        return;

    glfwSetCursor(m_handle, m_cursors[static_cast<size_t>(icon)]);
    m_cursor_icon = icon;
}

char const* platform_win64::glfw_window::get_clipboard_text()
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
    if (auto button = get_key(key))
        m_on_keyboard_button(get_action(action), *button, get_modifiers(mods));
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

void platform_win64::glfw_window::on_scroll(double xoffset, double yoffset)
{
    m_on_scroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void platform_win64::glfw_window::on_char(unsigned int codepoint)
{
    m_on_char(codepoint);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

namespace
{
    constexpr wchar_t const* WINDOW_CLASS_NAME = L"vulkan_renderer_demo";

    constexpr DWORD WINDOW_STYLE = WS_OVERLAPPEDWINDOW | WS_CAPTION;
    constexpr DWORD WINDOW_STYLE_EX = 0;

    class wstring
    {
    public:
        wstring(nstl::any_allocator alloc = {}) : wstring(0, nstl::move(alloc)) {}
        wstring(size_t length, nstl::any_allocator alloc = {})
            : m_buffer(length, sizeof(wchar_t), nstl::move(alloc))
        {
            m_buffer.resize(length);
        }

        size_t length() const { return m_buffer.size(); }
        wchar_t const* c_str() const { return data(); }
        wchar_t* data() { return m_buffer.get<wchar_t>(0); }
        wchar_t const* data() const { return m_buffer.get<wchar_t>(0); }

        wchar_t& operator[](size_t index) { assert(index < m_buffer.size()); return *m_buffer.get<wchar_t>(index); }

    private:
        nstl::buffer m_buffer;
    };

    wstring convert_to_wstring(nstl::string_view str)
    {
        if (str.empty())
            return {};

        int length = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), nullptr, 0);
        assert(length > 0);

        wstring result{ static_cast<size_t>(length) + 1 };

        int chars_written = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), result.data(), result.length());
        assert(chars_written == length);

        result[chars_written] = 0;

        return result;
    }

    nstl::string convert_to_string(WCHAR const* str)
    {
        int length = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
        if (length == 0)
            return {};

        nstl::string result{ static_cast<size_t>(length) };

        int chars_written = WideCharToMultiByte(CP_UTF8, 0, str, -1, result.data(), result.length(), nullptr, nullptr);
        assert(chars_written == length);

        return result;
    }

    template<typename T>
    void set_window_extra(HWND handle, T const& extra)
    {
        static_assert(nstl::is_trivially_copyable_v<T>);

        static_assert(sizeof(T) <= sizeof(LONG_PTR));
        LONG_PTR representation{};
        memcpy(&representation, &extra, sizeof(extra));

        SetWindowLongPtrW(handle, 0, representation);
    }

    template<typename T>
    T get_window_extra(HWND handle)
    {
        static_assert(nstl::is_trivially_copyable_v<T>);

        LONG_PTR representation = GetWindowLongPtrW(handle, 0);

        T extra;
        static_assert(sizeof(T) <= sizeof(LONG_PTR));
        memcpy(&extra, &representation, sizeof(extra));
        return extra;
    }

    nstl::optional<platform::keyboard_button> translate_key(WPARAM wparam, LPARAM lparam)
    {
        WORD vk = LOWORD(wparam);
        WORD flags = HIWORD(lparam);

        WORD scancode = LOBYTE(flags);
        bool is_extended = (flags & KF_EXTENDED) == KF_EXTENDED;

        if (is_extended)
            scancode = MAKEWORD(scancode, 0xE0);

        if (vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU)
            vk = LOWORD(MapVirtualKeyW(scancode, MAPVK_VSC_TO_VK_EX));

        switch (vk)
        {
        case VK_TAB: return platform::keyboard_button::tab;
        case VK_LEFT: return platform::keyboard_button::left_arrow;
        case VK_RIGHT: return platform::keyboard_button::right_arrow;
        case VK_UP: return platform::keyboard_button::up_arrow;
        case VK_DOWN: return platform::keyboard_button::down_arrow;
        case VK_PRIOR: return platform::keyboard_button::page_up;
        case VK_NEXT: return platform::keyboard_button::page_down;
        case VK_HOME: return platform::keyboard_button::home;
        case VK_END: return platform::keyboard_button::end;
        case VK_INSERT: return platform::keyboard_button::insert;
        case VK_DELETE: return platform::keyboard_button::del;
        case VK_BACK: return platform::keyboard_button::backspace;
        case VK_SPACE: return platform::keyboard_button::space;
        case VK_RETURN: return platform::keyboard_button::enter;
        case VK_ESCAPE: return platform::keyboard_button::escape;
        case VK_OEM_7: return platform::keyboard_button::apostrophe;
        case VK_OEM_COMMA: return platform::keyboard_button::comma;
        case VK_OEM_MINUS: return platform::keyboard_button::minus;
        case VK_OEM_PERIOD: return platform::keyboard_button::period;
        case VK_OEM_2: return platform::keyboard_button::slash;
        case VK_OEM_1: return platform::keyboard_button::semicolon;
        case VK_OEM_PLUS: return platform::keyboard_button::equal;
        case VK_OEM_4: return platform::keyboard_button::left_bracket;
        case VK_OEM_5: return platform::keyboard_button::backslash;
        case VK_OEM_6: return platform::keyboard_button::right_bracket;
        case VK_OEM_3: return platform::keyboard_button::grave_accent;
        case VK_CAPITAL: return platform::keyboard_button::caps_lock;
        case VK_SCROLL: return platform::keyboard_button::scroll_lock;
        case VK_NUMLOCK: return platform::keyboard_button::num_lock;
        case VK_SNAPSHOT: return platform::keyboard_button::print_screen;
        case VK_PAUSE: return platform::keyboard_button::pause;
        case VK_NUMPAD0: return platform::keyboard_button::keypad0;
        case VK_NUMPAD1: return platform::keyboard_button::keypad1;
        case VK_NUMPAD2: return platform::keyboard_button::keypad2;
        case VK_NUMPAD3: return platform::keyboard_button::keypad3;
        case VK_NUMPAD4: return platform::keyboard_button::keypad4;
        case VK_NUMPAD5: return platform::keyboard_button::keypad5;
        case VK_NUMPAD6: return platform::keyboard_button::keypad6;
        case VK_NUMPAD7: return platform::keyboard_button::keypad7;
        case VK_NUMPAD8: return platform::keyboard_button::keypad8;
        case VK_NUMPAD9: return platform::keyboard_button::keypad9;
        case VK_DECIMAL: return platform::keyboard_button::keypad_decimal;
        case VK_DIVIDE: return platform::keyboard_button::keypad_divide;
        case VK_MULTIPLY: return platform::keyboard_button::keypad_multiply;
        case VK_SUBTRACT: return platform::keyboard_button::keypad_subtract;
        case VK_ADD: return platform::keyboard_button::keypad_add;
        case VK_LSHIFT: return platform::keyboard_button::left_shift;
        case VK_LCONTROL: return platform::keyboard_button::left_ctrl;
        case VK_LMENU: return platform::keyboard_button::left_alt;
        case VK_LWIN: return platform::keyboard_button::left_super;
        case VK_RSHIFT: return platform::keyboard_button::right_shift;
        case VK_RCONTROL: return platform::keyboard_button::right_ctrl;
        case VK_RMENU: return platform::keyboard_button::right_alt;
        case VK_RWIN: return platform::keyboard_button::right_super;
        case VK_MENU: return platform::keyboard_button::menu;
        case 0x30: return platform::keyboard_button::zero;
        case 0x31: return platform::keyboard_button::one;
        case 0x32: return platform::keyboard_button::two;
        case 0x33: return platform::keyboard_button::three;
        case 0x34: return platform::keyboard_button::four;
        case 0x35: return platform::keyboard_button::five;
        case 0x36: return platform::keyboard_button::six;
        case 0x37: return platform::keyboard_button::seven;
        case 0x38: return platform::keyboard_button::eight;
        case 0x39: return platform::keyboard_button::nine;
        case 0x41: return platform::keyboard_button::a;
        case 0x42: return platform::keyboard_button::b;
        case 0x43: return platform::keyboard_button::c;
        case 0x44: return platform::keyboard_button::d;
        case 0x45: return platform::keyboard_button::e;
        case 0x46: return platform::keyboard_button::f;
        case 0x47: return platform::keyboard_button::g;
        case 0x48: return platform::keyboard_button::h;
        case 0x49: return platform::keyboard_button::i;
        case 0x4a: return platform::keyboard_button::j;
        case 0x4b: return platform::keyboard_button::k;
        case 0x4c: return platform::keyboard_button::l;
        case 0x4d: return platform::keyboard_button::m;
        case 0x4e: return platform::keyboard_button::n;
        case 0x4f: return platform::keyboard_button::o;
        case 0x50: return platform::keyboard_button::p;
        case 0x51: return platform::keyboard_button::q;
        case 0x52: return platform::keyboard_button::r;
        case 0x53: return platform::keyboard_button::s;
        case 0x54: return platform::keyboard_button::t;
        case 0x55: return platform::keyboard_button::u;
        case 0x56: return platform::keyboard_button::v;
        case 0x57: return platform::keyboard_button::w;
        case 0x58: return platform::keyboard_button::x;
        case 0x59: return platform::keyboard_button::y;
        case 0x5a: return platform::keyboard_button::z;
        case VK_F1: return platform::keyboard_button::f1;
        case VK_F2: return platform::keyboard_button::f2;
        case VK_F3: return platform::keyboard_button::f3;
        case VK_F4: return platform::keyboard_button::f4;
        case VK_F5: return platform::keyboard_button::f5;
        case VK_F6: return platform::keyboard_button::f6;
        case VK_F7: return platform::keyboard_button::f7;
        case VK_F8: return platform::keyboard_button::f8;
        case VK_F9: return platform::keyboard_button::f9;
        case VK_F10: return platform::keyboard_button::f10;
        case VK_F11: return platform::keyboard_button::f11;
        case VK_F12: return platform::keyboard_button::f12;
        }

        return {};
    }

    LPCTSTR get_cursor_id(platform::mouse_cursor_icon type)
    {
        switch (type)
        {
        case platform::mouse_cursor_icon::arrow: return IDC_ARROW;
        case platform::mouse_cursor_icon::text_input: return IDC_IBEAM;
        case platform::mouse_cursor_icon::resize_all: return IDC_SIZEALL;
        case platform::mouse_cursor_icon::resize_ns: return IDC_SIZENS;
        case platform::mouse_cursor_icon::resize_ew: return IDC_SIZEWE;
        case platform::mouse_cursor_icon::resize_nesw: return IDC_SIZENESW;
        case platform::mouse_cursor_icon::resize_nwse: return IDC_SIZENWSE;
        case platform::mouse_cursor_icon::not_allowed: return IDC_NO;
        case platform::mouse_cursor_icon::hand: return IDC_HAND;
        }

        assert(false);
        return {};
    }

    LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        platform_win64::win32_window* window = get_window_extra<platform_win64::win32_window*>(hwnd);

        switch (msg)
        {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if ((HIWORD(lparam) & KF_REPEAT) == KF_REPEAT)
                break;
            if (auto key = translate_key(wparam, lparam))
                window->on_keyboard_button(platform::button_action::press, *key);
            break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (auto key = translate_key(wparam, lparam))
                window->on_keyboard_button(platform::button_action::release, *key);
            break;

        case WM_CHAR:
        case WM_SYSCHAR:
            window->on_char(wparam);
            break;

        case WM_MOUSEMOVE:
            window->on_cursor_position(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            break;

        case WM_LBUTTONDOWN:
            window->on_mouse_button(platform::button_action::press, platform::mouse_button::left);
            break;
        case WM_RBUTTONDOWN:
            window->on_mouse_button(platform::button_action::press, platform::mouse_button::right);
            break;
        case WM_MBUTTONDOWN:
            window->on_mouse_button(platform::button_action::press, platform::mouse_button::middle);
            break;
        case WM_LBUTTONUP:
            window->on_mouse_button(platform::button_action::release, platform::mouse_button::left);
            break;
        case WM_RBUTTONUP:
            window->on_mouse_button(platform::button_action::release, platform::mouse_button::right);
            break;
        case WM_MBUTTONUP:
            window->on_mouse_button(platform::button_action::release, platform::mouse_button::middle);
            break;

        case WM_MOUSEWHEEL:
            window->on_scroll(0.0f, 1.0f * GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA);
            break;
        case WM_MOUSEHWHEEL:
            window->on_scroll(1.0f * GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA, 0.0f);
            break;

        case WM_SIZE:
            window->on_window_resized(LOWORD(lparam), HIWORD(lparam), wparam == SIZE_MINIMIZED);
            break;

        case WM_SETFOCUS:
            window->on_focus(true);
            break;
        case WM_KILLFOCUS:
            window->on_focus(false);
            break;

        case WM_SETCURSOR:
            if (LOWORD(lparam) == HTCLIENT)
            {
                window->update_cursor();
                return TRUE;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        }

        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
}

platform_win64::win32_window::win32_window(size_t width, size_t height, char const* title)
{
    HINSTANCE instance = GetModuleHandle(nullptr);

    WNDCLASSEXW wc = {
        .cbSize = sizeof(wc),
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = &::window_proc,
        .cbWndExtra = sizeof(platform_win64::win32_window*),
        .hInstance = instance,
        .hIcon = LoadIcon(nullptr, IDI_APPLICATION),
        .hCursor = LoadCursor(nullptr, IDC_ARROW),
        .lpszClassName = WINDOW_CLASS_NAME,
    };

    if (!RegisterClassExW(&wc))
        assert(false);

    wstring wide_title = convert_to_wstring(title);

    RECT rect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
    AdjustWindowRectExForDpi(&rect, WINDOW_STYLE, FALSE, WINDOW_STYLE_EX, USER_DEFAULT_SCREEN_DPI);
    int full_width = rect.right - rect.left;
    int full_height = rect.bottom - rect.top;

    HWND& handle = m_handle.get_as<HWND>();
    handle = CreateWindowExW(WINDOW_STYLE_EX, WINDOW_CLASS_NAME, wide_title.data(), WINDOW_STYLE, CW_USEDEFAULT, CW_USEDEFAULT, full_width, full_height, nullptr, nullptr, instance, nullptr);
    assert(handle);

    set_window_extra<platform_win64::win32_window*>(handle, this);

    ShowWindow(handle, SW_SHOW);
    UpdateWindow(handle);

    m_window_width = width;
    m_window_height = height;
    m_framebuffer_width = width;
    m_framebuffer_height = height;
}

platform_win64::win32_window::~win32_window()
{
    DestroyWindow(m_handle.get_as<HWND>());
    m_handle = {};

    UnregisterClassW(WINDOW_CLASS_NAME, GetModuleHandle(nullptr));
}

void platform_win64::win32_window::poll_events()
{
    if (m_is_closed)
        return;

    MSG msg{};

    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT)
        {
            m_is_closed = true;
            break;
        }
    }
}

bool platform_win64::win32_window::is_closed() const
{
    return m_is_closed;
}

void platform_win64::win32_window::resize(size_t width, size_t height)
{
    assert(width <= LONG_MAX);
    assert(height <= LONG_MAX);
    RECT rect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };

    AdjustWindowRectExForDpi(&rect, WINDOW_STYLE, FALSE, WINDOW_STYLE_EX, USER_DEFAULT_SCREEN_DPI);
    int full_width = rect.right - rect.left;
    int full_height = rect.bottom - rect.top;

    SetWindowPos(m_handle.get_as<HWND>(), HWND_TOP, 0, 0, full_width, full_height, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
}

void platform_win64::win32_window::set_cursor_mode(platform::mouse_cursor_mode mode)
{
    if (m_cursor_mode == mode)
        return;

    m_cursor_mode = mode;
    update_cursor();
}

void platform_win64::win32_window::set_cursor_icon(platform::mouse_cursor_icon icon)
{
    if (m_cursor_icon == icon)
        return;

    m_cursor_icon = icon;
    update_cursor();
}

char const* platform_win64::win32_window::get_clipboard_text()
{
    if (!OpenClipboard(m_handle.get_as<HWND>()))
    {
        assert(false);
        return nullptr;
    }

    HANDLE object = GetClipboardData(CF_UNICODETEXT);
    if (!object)
    {
        assert(false);
        CloseClipboard();
        return nullptr;
    }

    WCHAR* buffer = static_cast<WCHAR*>(GlobalLock(object));
    if (!buffer)
    {
        assert(false);
        CloseClipboard();
        return nullptr;
    }

    m_clipboard_text = convert_to_string(buffer);

    GlobalUnlock(object);
    CloseClipboard();

    return m_clipboard_text.c_str();
}

void platform_win64::win32_window::set_clipboard_text(char const* text)
{
    // TODO use wstring and convert_to_wstring

    int length = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
    if (length <= 0)
        return;

    HANDLE object = GlobalAlloc(GMEM_MOVEABLE, length * sizeof(WCHAR));
    if (!object)
    {
        assert(false);
        return;
    }

    WCHAR* buffer = static_cast<WCHAR*>(GlobalLock(object));
    if (!buffer)
    {
        assert(false);
        GlobalFree(object);
        return;
    }

    MultiByteToWideChar(CP_UTF8, 0, text, -1, buffer, length);
    GlobalUnlock(object);

    if (!OpenClipboard(m_handle.get_as<HWND>()))
    {
        assert(false);
        GlobalFree(object);
        return;
    }

    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, object);
    CloseClipboard();
}

void platform_win64::win32_window::on_window_resized(size_t width, size_t height, bool is_minimzed)
{
    m_window_width = width;
    m_window_height = height;
    m_framebuffer_width = width;
    m_framebuffer_height = height;
    m_is_iconified = is_minimzed;

    m_on_window_resize(width, height);
    m_on_framebuffer_resize(width, height);
}

void platform_win64::win32_window::on_keyboard_button(platform::button_action action, platform::keyboard_button button)
{
    auto update_modifier = [this](platform::button_action action, platform::button_modifiers modifier)
    {
        if (action == platform::button_action::press)
            m_current_modifiers |= modifier;
        else
            m_current_modifiers &= ~modifier;
    };

    bool is_press = action == platform::button_action::press;
    if (button == platform::keyboard_button::left_shift || button == platform::keyboard_button::right_shift)
        update_modifier(action, platform::button_modifiers::shift);
    if (button == platform::keyboard_button::left_ctrl || button == platform::keyboard_button::right_ctrl)
        update_modifier(action, platform::button_modifiers::ctrl);
    if (button == platform::keyboard_button::left_alt || button == platform::keyboard_button::right_alt)
        update_modifier(action, platform::button_modifiers::alt);

    m_on_keyboard_button(action, button, m_current_modifiers);
}

void platform_win64::win32_window::on_mouse_button(platform::button_action action, platform::mouse_button button)
{
    m_on_mouse_button(action, button, m_current_modifiers);
}

void platform_win64::win32_window::on_cursor_position(int xpos, int ypos)
{
    m_on_mouse_position(xpos, ypos);

    int dx = xpos - m_last_mouse_pos_x;
    int dy = ypos - m_last_mouse_pos_y;

    m_last_mouse_pos_x = xpos;
    m_last_mouse_pos_y = ypos;

    m_on_mouse_delta(dx, dy);
}

void platform_win64::win32_window::on_focus(bool focused)
{
    m_on_focus(focused);
}

void platform_win64::win32_window::on_scroll(float xoffset, float yoffset)
{
    m_on_scroll(xoffset, yoffset);
}

void platform_win64::win32_window::on_char(unsigned int codepoint)
{
    m_on_char(codepoint);
}

void platform_win64::win32_window::update_cursor()
{
    switch (m_cursor_mode)
    {
    case platform::mouse_cursor_mode::visible:
        SetCursor(LoadCursor(nullptr, get_cursor_id(m_cursor_icon)));
        break;
    case platform::mouse_cursor_mode::hidden:
    case platform::mouse_cursor_mode::disabled:
        SetCursor(nullptr);
        break;
    }
}
