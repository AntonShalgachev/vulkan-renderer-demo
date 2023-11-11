#pragma once

#include "platform/window.h"

#include "nstl/array.h"
#include "nstl/event.h"

struct GLFWwindow;
struct GLFWcursor;

namespace platform_win64
{
    class glfw_window : public platform::window
    {
    public:
        glfw_window(size_t width, size_t height, char const* title);
        ~glfw_window();

        GLFWwindow* get_handle() const { return m_handle; }

        void poll_events() override;

        void add_window_resize_callback(nstl::function<void(size_t, size_t)> callback) override { m_on_window_resize.add(nstl::move(callback)); }
        void add_framebuffer_resize_callback(nstl::function<void(size_t, size_t)> callback) override { m_on_framebuffer_resize.add(nstl::move(callback)); }
        void add_keyboard_button_callback(nstl::function<void(platform::button_action, platform::keyboard_button, platform::button_modifiers)> callback) override { m_on_keyboard_button.add(nstl::move(callback)); }
        void add_mouse_button_callback(nstl::function<void(platform::button_action, platform::mouse_button, platform::button_modifiers)> callback) override { m_on_mouse_button.add(nstl::move(callback)); }
        void add_mouse_position_callback(nstl::function<void(float, float)> callback) override { m_on_mouse_position.add(nstl::move(callback)); }
        void add_mouse_delta_callback(nstl::function<void(float, float)> callback) override { m_on_mouse_delta.add(nstl::move(callback)); }
        void add_focus_callback(nstl::function<void(bool)> callback) override { m_on_focus.add(nstl::move(callback)); }
        void add_cursor_enter_callback(nstl::function<void(bool)> callback) override { m_on_cursor_enter.add(nstl::move(callback)); }
        void add_scroll_callback(nstl::function<void(float, float)> callback) override { m_on_scroll.add(nstl::move(callback)); }
        void add_char_callback(nstl::function<void(char)> callback) override { m_on_char.add(nstl::move(callback)); }

        bool should_close() const override;
        bool is_iconified() const override { return m_is_iconified; }

        void resize(int width, int height) override;
        size_t get_window_width() const override { return m_window_width; }
        size_t get_window_height() const override { return m_window_height; }
        size_t get_framebuffer_width() const override { return m_framebuffer_width; }
        size_t get_framebuffer_height() const override { return m_framebuffer_height; }

        void set_cursor_mode(platform::mouse_cursor_mode mode) override;
        platform::mouse_cursor_mode get_cursor_mode() const override { return m_cursor_mode; }

        void set_cursor_icon(platform::mouse_cursor_icon icon) override;
        platform::mouse_cursor_icon get_cursor_icon() const override { return m_cursor_icon; }

        char const* get_clipboard_text() const override;
        void set_clipboard_text(char const* text) override;

    private:
        void on_window_resized(int width, int height);
        void on_framebuffer_resized(int width, int height);
        void on_iconified(int iconified);
        void on_keyboard_button(int key, int scancode, int action, int mods);
        void on_mouse_button(int button, int action, int mods);
        void on_cursor_position(double xpos, double ypos);
        void on_focus(int focused);
        void on_cursor_enter(int entered);
        void on_scroll(double xoffset, double yoffset);
        void on_char(unsigned int c);

    private:
        GLFWwindow* m_handle = nullptr;
        nstl::array<GLFWcursor*, tiny_ctti::enum_size_v<platform::mouse_cursor_icon>> m_cursors;

        bool m_is_iconified = false;

        size_t m_window_width = 0;
        size_t m_window_height = 0;
        size_t m_framebuffer_width = 0;
        size_t m_framebuffer_height = 0;

        float m_last_mouse_pos_x = 0.0f;
        float m_last_mouse_pos_y = 0.0f;

        platform::mouse_cursor_mode m_cursor_mode = platform::mouse_cursor_mode::normal;
        platform::mouse_cursor_icon m_cursor_icon = platform::mouse_cursor_icon::arrow;

        nstl::event<size_t, size_t> m_on_window_resize;
        nstl::event<size_t, size_t> m_on_framebuffer_resize;
        nstl::event<platform::button_action, platform::keyboard_button, platform::button_modifiers> m_on_keyboard_button;
        nstl::event<platform::button_action, platform::mouse_button, platform::button_modifiers> m_on_mouse_button;
        nstl::event<float, float> m_on_mouse_position;
        nstl::event<float, float> m_on_mouse_delta;
        nstl::event<bool> m_on_focus;
        nstl::event<bool> m_on_cursor_enter;
        nstl::event<float, float> m_on_scroll;
        nstl::event<char> m_on_char;
    };

    class window : public glfw_window
    {
        using glfw_window::glfw_window;
    };
}
