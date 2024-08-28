#include "platform_win64/window.h"

#include "common.h"

#include "nstl/string.h"

#include <Windowsx.h>

#include <limits.h>

namespace
{
    constexpr wchar_t const* WINDOW_CLASS_NAME = L"vulkan_renderer_demo";

    constexpr DWORD WINDOW_STYLE = WS_OVERLAPPEDWINDOW | WS_CAPTION;
    constexpr DWORD WINDOW_STYLE_EX = 0;

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
            assert(wparam >= 0);
            assert(wparam <= UINT_MAX);
            window->on_char(static_cast<unsigned int>(wparam));
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
    m_on_mouse_position(static_cast<float>(xpos), static_cast<float>(ypos));

    int dx = xpos - m_last_mouse_pos_x;
    int dy = ypos - m_last_mouse_pos_y;

    m_last_mouse_pos_x = xpos;
    m_last_mouse_pos_y = ypos;

    m_on_mouse_delta(static_cast<float>(dx), static_cast<float>(dy));
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
