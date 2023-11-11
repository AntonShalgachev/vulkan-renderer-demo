#include "ImGuiPlatform.h"

#include "memory/tracking.h"

#include "platform/window.h"

#include "imgui.h"

#include "nstl/enum.h"

namespace
{
    ImGuiMouseButton translate(platform::mouse_button button)
    {
        switch (button)
        {
        case platform::mouse_button::left:
            return ImGuiMouseButton_Left;
        case platform::mouse_button::middle:
            return ImGuiMouseButton_Middle;
        case platform::mouse_button::right:
            return ImGuiMouseButton_Right;
        }

        assert(false);
        return ImGuiMouseButton_Left;
    }

    ImGuiKey translate(platform::keyboard_button key)
    {
        switch (key)
        {
        case platform::keyboard_button::unknown: return ImGuiKey_None;
        case platform::keyboard_button::tab: return ImGuiKey_Tab;
        case platform::keyboard_button::left_arrow: return ImGuiKey_LeftArrow;
        case platform::keyboard_button::right_arrow: return ImGuiKey_RightArrow;
        case platform::keyboard_button::up_arrow: return ImGuiKey_UpArrow;
        case platform::keyboard_button::down_arrow: return ImGuiKey_DownArrow;
        case platform::keyboard_button::page_up: return ImGuiKey_PageUp;
        case platform::keyboard_button::page_down: return ImGuiKey_PageDown;
        case platform::keyboard_button::home: return ImGuiKey_Home;
        case platform::keyboard_button::end: return ImGuiKey_End;
        case platform::keyboard_button::insert: return ImGuiKey_Insert;
        case platform::keyboard_button::del: return ImGuiKey_Delete;
        case platform::keyboard_button::backspace: return ImGuiKey_Backspace;
        case platform::keyboard_button::space: return ImGuiKey_Space;
        case platform::keyboard_button::enter: return ImGuiKey_Enter;
        case platform::keyboard_button::escape: return ImGuiKey_Escape;
        case platform::keyboard_button::apostrophe: return ImGuiKey_Apostrophe;
        case platform::keyboard_button::comma: return ImGuiKey_Comma;
        case platform::keyboard_button::minus: return ImGuiKey_Minus;
        case platform::keyboard_button::period: return ImGuiKey_Period;
        case platform::keyboard_button::slash: return ImGuiKey_Slash;
        case platform::keyboard_button::semicolon: return ImGuiKey_Semicolon;
        case platform::keyboard_button::equal: return ImGuiKey_Equal;
        case platform::keyboard_button::left_bracket: return ImGuiKey_LeftBracket;
        case platform::keyboard_button::backslash: return ImGuiKey_Backslash;
        case platform::keyboard_button::right_bracket: return ImGuiKey_RightBracket;
        case platform::keyboard_button::grave_accent: return ImGuiKey_GraveAccent;
        case platform::keyboard_button::caps_lock: return ImGuiKey_CapsLock;
        case platform::keyboard_button::scroll_lock: return ImGuiKey_ScrollLock;
        case platform::keyboard_button::num_lock: return ImGuiKey_NumLock;
        case platform::keyboard_button::print_screen: return ImGuiKey_PrintScreen;
        case platform::keyboard_button::pause: return ImGuiKey_Pause;
        case platform::keyboard_button::keypad0: return ImGuiKey_Keypad0;
        case platform::keyboard_button::keypad1: return ImGuiKey_Keypad1;
        case platform::keyboard_button::keypad2: return ImGuiKey_Keypad2;
        case platform::keyboard_button::keypad3: return ImGuiKey_Keypad3;
        case platform::keyboard_button::keypad4: return ImGuiKey_Keypad4;
        case platform::keyboard_button::keypad5: return ImGuiKey_Keypad5;
        case platform::keyboard_button::keypad6: return ImGuiKey_Keypad6;
        case platform::keyboard_button::keypad7: return ImGuiKey_Keypad7;
        case platform::keyboard_button::keypad8: return ImGuiKey_Keypad8;
        case platform::keyboard_button::keypad9: return ImGuiKey_Keypad9;
        case platform::keyboard_button::keypad_decimal: return ImGuiKey_KeypadDecimal;
        case platform::keyboard_button::keypad_divide: return ImGuiKey_KeypadDivide;
        case platform::keyboard_button::keypad_multiply: return ImGuiKey_KeypadMultiply;
        case platform::keyboard_button::keypad_subtract: return ImGuiKey_KeypadSubtract;
        case platform::keyboard_button::keypad_add: return ImGuiKey_KeypadAdd;
        case platform::keyboard_button::keypad_enter: return ImGuiKey_KeypadEnter;
        case platform::keyboard_button::keypad_equal: return ImGuiKey_KeypadEqual;
        case platform::keyboard_button::left_shift: return ImGuiKey_LeftShift;
        case platform::keyboard_button::left_ctrl: return ImGuiKey_LeftCtrl;
        case platform::keyboard_button::left_alt: return ImGuiKey_LeftAlt;
        case platform::keyboard_button::left_super: return ImGuiKey_LeftSuper;
        case platform::keyboard_button::right_shift: return ImGuiKey_RightShift;
        case platform::keyboard_button::right_ctrl: return ImGuiKey_RightCtrl;
        case platform::keyboard_button::right_alt: return ImGuiKey_RightAlt;
        case platform::keyboard_button::right_super: return ImGuiKey_RightSuper;
        case platform::keyboard_button::menu: return ImGuiKey_Menu;
        case platform::keyboard_button::zero: return ImGuiKey_0;
        case platform::keyboard_button::one: return ImGuiKey_1;
        case platform::keyboard_button::two: return ImGuiKey_2;
        case platform::keyboard_button::three: return ImGuiKey_3;
        case platform::keyboard_button::four: return ImGuiKey_4;
        case platform::keyboard_button::five: return ImGuiKey_5;
        case platform::keyboard_button::six: return ImGuiKey_6;
        case platform::keyboard_button::seven: return ImGuiKey_7;
        case platform::keyboard_button::eight: return ImGuiKey_8;
        case platform::keyboard_button::nine: return ImGuiKey_9;
        case platform::keyboard_button::a: return ImGuiKey_A;
        case platform::keyboard_button::b: return ImGuiKey_B;
        case platform::keyboard_button::c: return ImGuiKey_C;
        case platform::keyboard_button::d: return ImGuiKey_D;
        case platform::keyboard_button::e: return ImGuiKey_E;
        case platform::keyboard_button::f: return ImGuiKey_F;
        case platform::keyboard_button::g: return ImGuiKey_G;
        case platform::keyboard_button::h: return ImGuiKey_H;
        case platform::keyboard_button::i: return ImGuiKey_I;
        case platform::keyboard_button::j: return ImGuiKey_J;
        case platform::keyboard_button::k: return ImGuiKey_K;
        case platform::keyboard_button::l: return ImGuiKey_L;
        case platform::keyboard_button::m: return ImGuiKey_M;
        case platform::keyboard_button::n: return ImGuiKey_N;
        case platform::keyboard_button::o: return ImGuiKey_O;
        case platform::keyboard_button::p: return ImGuiKey_P;
        case platform::keyboard_button::q: return ImGuiKey_Q;
        case platform::keyboard_button::r: return ImGuiKey_R;
        case platform::keyboard_button::s: return ImGuiKey_S;
        case platform::keyboard_button::t: return ImGuiKey_T;
        case platform::keyboard_button::u: return ImGuiKey_U;
        case platform::keyboard_button::v: return ImGuiKey_V;
        case platform::keyboard_button::w: return ImGuiKey_W;
        case platform::keyboard_button::x: return ImGuiKey_X;
        case platform::keyboard_button::y: return ImGuiKey_Y;
        case platform::keyboard_button::z: return ImGuiKey_Z;
        case platform::keyboard_button::f1: return ImGuiKey_F1;
        case platform::keyboard_button::f2: return ImGuiKey_F2;
        case platform::keyboard_button::f3: return ImGuiKey_F3;
        case platform::keyboard_button::f4: return ImGuiKey_F4;
        case platform::keyboard_button::f5: return ImGuiKey_F5;
        case platform::keyboard_button::f6: return ImGuiKey_F6;
        case platform::keyboard_button::f7: return ImGuiKey_F7;
        case platform::keyboard_button::f8: return ImGuiKey_F8;
        case platform::keyboard_button::f9: return ImGuiKey_F9;
        case platform::keyboard_button::f10: return ImGuiKey_F10;
        case platform::keyboard_button::f11: return ImGuiKey_F11;
        case platform::keyboard_button::f12: return ImGuiKey_F12;
        }

        assert(false);
        return ImGuiKey_None;
    }

    platform::mouse_cursor_icon translate(ImGuiMouseCursor cursor)
    {
        switch (cursor)
        {
        case ImGuiMouseCursor_Arrow: return platform::mouse_cursor_icon::arrow;
        case ImGuiMouseCursor_TextInput: return platform::mouse_cursor_icon::text_input;
        case ImGuiMouseCursor_ResizeAll: return platform::mouse_cursor_icon::resize_all;
        case ImGuiMouseCursor_ResizeNS: return platform::mouse_cursor_icon::resize_ns;
        case ImGuiMouseCursor_ResizeEW: return platform::mouse_cursor_icon::resize_ew;
        case ImGuiMouseCursor_ResizeNESW: return platform::mouse_cursor_icon::resize_nesw;
        case ImGuiMouseCursor_ResizeNWSE: return platform::mouse_cursor_icon::resize_nwse;
        case ImGuiMouseCursor_NotAllowed: return platform::mouse_cursor_icon::not_allowed;
        case ImGuiMouseCursor_Hand: return platform::mouse_cursor_icon::hand;
        }

        assert(false);
        return platform::mouse_cursor_icon::arrow;
    }

    void updateModifiers(platform::button_modifiers modifiers)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddKeyEvent(ImGuiKey_ModCtrl, (modifiers & platform::button_modifiers::ctrl) != 0);
        io.AddKeyEvent(ImGuiKey_ModShift, (modifiers & platform::button_modifiers::shift) != 0);
        io.AddKeyEvent(ImGuiKey_ModAlt, (modifiers & platform::button_modifiers::alt) != 0);
    }
}

ImGuiPlatform::ImGuiPlatform(platform::window& window) : m_window(window)
{
    static auto scopeId = memory::tracking::create_scope_id("UI/ImGui/Platform");
    MEMORY_TRACKING_SCOPE(scopeId);

    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = "vulkan_renderer_demo";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

    io.SetClipboardTextFn = [](void* userData, char const* text) { return static_cast<platform::window*>(userData)->set_clipboard_text(text); };
    io.GetClipboardTextFn = [](void* userData) { return static_cast<platform::window*>(userData)->get_clipboard_text(); };
    io.ClipboardUserData = &m_window;

    setupCallbacks();
}

ImGuiPlatform::~ImGuiPlatform()
{

}

void ImGuiPlatform::update(float frameTime)
{
    ImGuiIO& io = ImGui::GetIO();

    updateDisplaySize();
    updateCursor();

    io.DeltaTime = frameTime > 0.0f ? frameTime : 0.16f;
}

void ImGuiPlatform::setupCallbacks()
{
    m_window.add_focus_callback([](bool focused) {
        ImGui::GetIO().AddFocusEvent(focused);
    });

    m_window.add_mouse_button_callback([](platform::button_action action, platform::mouse_button button, platform::button_modifiers modifiers) {
        if (action != platform::button_action::press && action != platform::button_action::release)
            return;

        updateModifiers(modifiers);
        ImGui::GetIO().AddMouseButtonEvent(translate(button), action == platform::button_action::press);
    });

    m_window.add_mouse_position_callback([](float x, float y) {
        ImGui::GetIO().AddMousePosEvent(x, y);
    });

    m_window.add_keyboard_button_callback([](platform::button_action action, platform::keyboard_button button, platform::button_modifiers modifiers) {
        if (action != platform::button_action::press && action != platform::button_action::release)
            return;

        updateModifiers(modifiers);

//         keycode = ImGui_ImplGlfw_TranslateUntranslatedKey(keycode, scancode); // TODO

        ImGui::GetIO().AddKeyEvent(translate(button), action == platform::button_action::press);
    });

    m_window.add_char_callback([](char c) {
        assert(c >= 0);
        ImGui::GetIO().AddInputCharacter(static_cast<unsigned int>(c));
    });

    m_window.add_scroll_callback([](float x, float y) {
        ImGui::GetIO().AddMouseWheelEvent(x, y);
    });
}

void ImGuiPlatform::updateDisplaySize()
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowSize = { 1.0f * m_window.get_window_width(), 1.0f * m_window.get_window_height() };
    ImVec2 framebufferSize = { 1.0f * m_window.get_framebuffer_width(), 1.0f * m_window.get_framebuffer_height() };
    io.DisplaySize = windowSize;
    if (windowSize.x > 0 && windowSize.y > 0)
        io.DisplayFramebufferScale = ImVec2(framebufferSize.x / windowSize.x, framebufferSize.y / windowSize.y);
}

void ImGuiPlatform::updateCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange))
        return;

    if (m_window.get_cursor_mode() == platform::mouse_cursor_mode::disabled)
        return;

    ImGuiMouseCursor imguiCursor = ImGui::GetMouseCursor();
    if (imguiCursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
    {
        m_window.set_cursor_mode(platform::mouse_cursor_mode::hidden);
    }
    else
    {
        m_window.set_cursor_mode(platform::mouse_cursor_mode::normal);
        m_window.set_cursor_icon(translate(imguiCursor));
    }
}
