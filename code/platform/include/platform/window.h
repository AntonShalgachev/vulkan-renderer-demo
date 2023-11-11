#pragma once

#include "tiny_ctti/tiny_ctti.hpp"

#include "nstl/function.h"

namespace platform
{
    using window_handle_t = void*;

    // TODO separate window from input

    enum class mouse_cursor_mode
    {
        normal,
        hidden,
        disabled,
    };
    TINY_CTTI_DESCRIBE_ENUM(mouse_cursor_mode, normal, hidden, disabled);

    enum class mouse_cursor_icon
    {
        arrow,
        text_input,
        resize_all,
        resize_ns,
        resize_ew,
        resize_nesw,
        resize_nwse,
        not_allowed,
        hand,
    };
    TINY_CTTI_DESCRIBE_ENUM(mouse_cursor_icon, arrow, text_input, resize_all, resize_ns, resize_ew, resize_nesw, resize_nwse, not_allowed, hand);

    enum class keyboard_button
    {
        unknown,
        tab, left_arrow, right_arrow, up_arrow, down_arrow,
        page_up, page_down, home, end, insert, del,
        backspace, space, enter, escape, apostrophe, comma,
        minus, period, slash, semicolon, equal, menu,
        left_bracket, backslash, right_bracket, grave_accent,
        caps_lock, scroll_lock, num_lock, print_screen, pause,
        keypad0, keypad1, keypad2, keypad3, keypad4, keypad5, keypad6, keypad7, keypad8, keypad9,
        keypad_decimal, keypad_divide, keypad_multiply, keypad_subtract, keypad_add, keypad_enter, keypad_equal,
        left_shift, left_ctrl, left_alt, left_super, right_shift, right_ctrl, right_alt, right_super,
        zero, one, two, three, four, five, six, seven, eight, nine,
        a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
        f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12
    };
    TINY_CTTI_DESCRIBE_ENUM(keyboard_button,
        unknown,
        tab, left_arrow, right_arrow, up_arrow, down_arrow,
        page_up, page_down, home, end, insert, del,
        backspace, space, enter, escape, apostrophe, comma,
        minus, period, slash, semicolon, equal, menu,
        left_bracket, backslash, right_bracket, grave_accent,
        caps_lock, scroll_lock, num_lock, print_screen, pause,
        keypad0, keypad1, keypad2, keypad3, keypad4, keypad5, keypad6, keypad7, keypad8, keypad9,
        keypad_decimal, keypad_divide, keypad_multiply, keypad_subtract, keypad_add, keypad_enter, keypad_equal,
        left_shift, left_ctrl, left_alt, left_super, right_shift, right_ctrl, right_alt, right_super,
        zero, one, two, three, four, five, six, seven, eight, nine,
        a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
        f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12
    );

    enum class mouse_button
    {
        left,
        middle,
        right,
    };
    TINY_CTTI_DESCRIBE_ENUM(mouse_button, left, middle, right);

    enum class button_modifiers
    {
        none = 0,
        ctrl = 1,
        shift = 2,
        alt = 4,
    };
    TINY_CTTI_DESCRIBE_ENUM(button_modifiers, none, ctrl, shift, alt);

    enum class button_action
    {
        press,
        release,
        repeat,
    };
    TINY_CTTI_DESCRIBE_ENUM(button_action, press, release, repeat);

    class window
    {
    public:
        virtual ~window() = default;

        virtual void poll_events() = 0;

        virtual void add_window_resize_callback(nstl::function<void(size_t, size_t)> callback) = 0;
        virtual void add_framebuffer_resize_callback(nstl::function<void(size_t, size_t)> callback) = 0;
        virtual void add_keyboard_button_callback(nstl::function<void(button_action, keyboard_button, button_modifiers)> callback) = 0;
        virtual void add_mouse_button_callback(nstl::function<void(button_action, mouse_button, button_modifiers)> callback) = 0;
        virtual void add_mouse_position_callback(nstl::function<void(float, float)> callback) = 0;
        virtual void add_mouse_delta_callback(nstl::function<void(float, float)> callback) = 0;
        virtual void add_focus_callback(nstl::function<void(bool)> callback) = 0;
        virtual void add_cursor_enter_callback(nstl::function<void(bool)> callback) = 0;
        virtual void add_scroll_callback(nstl::function<void(float, float)> callback) = 0;
        virtual void add_char_callback(nstl::function<void(char)> callback) = 0;

        virtual bool should_close() const = 0;
        virtual bool is_iconified() const = 0;

        virtual void resize(int width, int height) = 0;
        virtual size_t get_window_width() const = 0;
        virtual size_t get_window_height() const = 0;
        virtual size_t get_framebuffer_width() const = 0;
        virtual size_t get_framebuffer_height() const = 0;

        virtual void set_cursor_mode(mouse_cursor_mode mode) = 0;
        virtual mouse_cursor_mode get_cursor_mode() const = 0;

        virtual void set_cursor_icon(mouse_cursor_icon icon) = 0;
        virtual mouse_cursor_icon get_cursor_icon() const = 0;

        // TODO use nstl::string_view?
        virtual char const* get_clipboard_text() const = 0;
        virtual void set_clipboard_text(char const* text) = 0;
    };
}
