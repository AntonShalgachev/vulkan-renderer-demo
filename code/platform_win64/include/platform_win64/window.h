#pragma once

#include "platform/window.h"

#include "nstl/aligned_storage.h"
#include "nstl/array.h"
#include "nstl/event.h"

namespace platform_win64
{
    class win32_window : public platform::window
    {
    public:
        using hwnd_t = nstl::aligned_storage_t<8, 8>;

        win32_window(size_t width, size_t height, char const* title);
        ~win32_window();

        hwnd_t get_handle() const { return m_handle; }

        void poll_events() override;

        void add_window_resize_callback(nstl::function<void(size_t, size_t)> callback) override { m_on_window_resize.add(nstl::move(callback)); }
        void add_framebuffer_resize_callback(nstl::function<void(size_t, size_t)> callback) override { m_on_framebuffer_resize.add(nstl::move(callback)); }
        void add_keyboard_button_callback(nstl::function<void(platform::button_action, platform::keyboard_button, platform::button_modifiers)> callback) override { m_on_keyboard_button.add(nstl::move(callback)); }
        void add_mouse_button_callback(nstl::function<void(platform::button_action, platform::mouse_button, platform::button_modifiers)> callback) override { m_on_mouse_button.add(nstl::move(callback)); }
        void add_mouse_position_callback(nstl::function<void(float, float)> callback) override { m_on_mouse_position.add(nstl::move(callback)); }
        void add_mouse_delta_callback(nstl::function<void(float, float)> callback) override { m_on_mouse_delta.add(nstl::move(callback)); }
        void add_focus_callback(nstl::function<void(bool)> callback) override { m_on_focus.add(nstl::move(callback)); }
        void add_scroll_callback(nstl::function<void(float, float)> callback) override { m_on_scroll.add(nstl::move(callback)); }
        void add_char_callback(nstl::function<void(unsigned int)> callback) override { m_on_char.add(nstl::move(callback)); }

        bool is_closed() const override;
        bool is_iconified() const override { return m_is_iconified; }

        void resize(size_t width, size_t height) override;
        size_t get_window_width() const override { return m_window_width; }
        size_t get_window_height() const override { return m_window_height; }
        size_t get_framebuffer_width() const override { return m_framebuffer_width; }
        size_t get_framebuffer_height() const override { return m_framebuffer_height; }

        void set_cursor_mode(platform::mouse_cursor_mode mode) override;
        platform::mouse_cursor_mode get_cursor_mode() const override { return m_cursor_mode; }

        void set_cursor_icon(platform::mouse_cursor_icon icon) override;
        platform::mouse_cursor_icon get_cursor_icon() const override { return m_cursor_icon; }

        char const* get_clipboard_text() override;
        void set_clipboard_text(char const* text) override;

    public:
        void on_window_resized(size_t width, size_t height, bool is_minimzed);
        void on_keyboard_button(platform::button_action action, platform::keyboard_button button);
        void on_mouse_button(platform::button_action action, platform::mouse_button button);
        void on_cursor_position(int xpos, int ypos);
        void on_focus(bool focused);
        void on_scroll(float xoffset, float yoffset);
        void on_char(unsigned int codepoint);

        void update_cursor();

    private:
        hwnd_t m_handle{};

        bool m_is_closed = false;
        bool m_is_iconified = false;

        size_t m_window_width = 0;
        size_t m_window_height = 0;
        size_t m_framebuffer_width = 0;
        size_t m_framebuffer_height = 0;

        int m_last_mouse_pos_x = 0.0f;
        int m_last_mouse_pos_y = 0.0f;

        platform::button_modifiers m_current_modifiers = platform::button_modifiers::none;
        nstl::string m_clipboard_text;

        platform::mouse_cursor_mode m_cursor_mode = platform::mouse_cursor_mode::visible;
        platform::mouse_cursor_icon m_cursor_icon = platform::mouse_cursor_icon::arrow;

        nstl::event<size_t, size_t> m_on_window_resize;
        nstl::event<size_t, size_t> m_on_framebuffer_resize;
        nstl::event<platform::button_action, platform::keyboard_button, platform::button_modifiers> m_on_keyboard_button;
        nstl::event<platform::button_action, platform::mouse_button, platform::button_modifiers> m_on_mouse_button;
        nstl::event<float, float> m_on_mouse_position;
        nstl::event<float, float> m_on_mouse_delta;
        nstl::event<bool> m_on_focus;
        nstl::event<float, float> m_on_scroll;
        nstl::event<unsigned int> m_on_char;
    };

    using window = win32_window;
}
