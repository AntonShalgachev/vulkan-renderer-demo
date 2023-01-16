#include "ImGuiPlatform.h"

#include "GlfwWindow.h"

#include "memory/tracking.h"

#include "imgui.h"

namespace
{
    ImGuiMouseButton translate(GlfwWindow::MouseButton button)
    {
        switch (button)
        {
        case GlfwWindow::MouseButton::Left:
            return ImGuiMouseButton_Left;
        case GlfwWindow::MouseButton::Middle:
            return ImGuiMouseButton_Middle;
        case GlfwWindow::MouseButton::Right:
            return ImGuiMouseButton_Right;
        }

        assert(false);
        return ImGuiMouseButton_Left;
    }

    ImGuiKey translate(GlfwWindow::Key key)
    {
        switch (key)
        {
        case GlfwWindow::Key::Unknown: return ImGuiKey_None;
        case GlfwWindow::Key::Tab: return ImGuiKey_Tab;
        case GlfwWindow::Key::LeftArrow: return ImGuiKey_LeftArrow;
        case GlfwWindow::Key::RightArrow: return ImGuiKey_RightArrow;
        case GlfwWindow::Key::UpArrow: return ImGuiKey_UpArrow;
        case GlfwWindow::Key::DownArrow: return ImGuiKey_DownArrow;
        case GlfwWindow::Key::PageUp: return ImGuiKey_PageUp;
        case GlfwWindow::Key::PageDown: return ImGuiKey_PageDown;
        case GlfwWindow::Key::Home: return ImGuiKey_Home;
        case GlfwWindow::Key::End: return ImGuiKey_End;
        case GlfwWindow::Key::Insert: return ImGuiKey_Insert;
        case GlfwWindow::Key::Delete: return ImGuiKey_Delete;
        case GlfwWindow::Key::Backspace: return ImGuiKey_Backspace;
        case GlfwWindow::Key::Space: return ImGuiKey_Space;
        case GlfwWindow::Key::Enter: return ImGuiKey_Enter;
        case GlfwWindow::Key::Escape: return ImGuiKey_Escape;
        case GlfwWindow::Key::Apostrophe: return ImGuiKey_Apostrophe;
        case GlfwWindow::Key::Comma: return ImGuiKey_Comma;
        case GlfwWindow::Key::Minus: return ImGuiKey_Minus;
        case GlfwWindow::Key::Period: return ImGuiKey_Period;
        case GlfwWindow::Key::Slash: return ImGuiKey_Slash;
        case GlfwWindow::Key::Semicolon: return ImGuiKey_Semicolon;
        case GlfwWindow::Key::Equal: return ImGuiKey_Equal;
        case GlfwWindow::Key::LeftBracket: return ImGuiKey_LeftBracket;
        case GlfwWindow::Key::Backslash: return ImGuiKey_Backslash;
        case GlfwWindow::Key::RightBracket: return ImGuiKey_RightBracket;
        case GlfwWindow::Key::GraveAccent: return ImGuiKey_GraveAccent;
        case GlfwWindow::Key::CapsLock: return ImGuiKey_CapsLock;
        case GlfwWindow::Key::ScrollLock: return ImGuiKey_ScrollLock;
        case GlfwWindow::Key::NumLock: return ImGuiKey_NumLock;
        case GlfwWindow::Key::PrintScreen: return ImGuiKey_PrintScreen;
        case GlfwWindow::Key::Pause: return ImGuiKey_Pause;
        case GlfwWindow::Key::Keypad0: return ImGuiKey_Keypad0;
        case GlfwWindow::Key::Keypad1: return ImGuiKey_Keypad1;
        case GlfwWindow::Key::Keypad2: return ImGuiKey_Keypad2;
        case GlfwWindow::Key::Keypad3: return ImGuiKey_Keypad3;
        case GlfwWindow::Key::Keypad4: return ImGuiKey_Keypad4;
        case GlfwWindow::Key::Keypad5: return ImGuiKey_Keypad5;
        case GlfwWindow::Key::Keypad6: return ImGuiKey_Keypad6;
        case GlfwWindow::Key::Keypad7: return ImGuiKey_Keypad7;
        case GlfwWindow::Key::Keypad8: return ImGuiKey_Keypad8;
        case GlfwWindow::Key::Keypad9: return ImGuiKey_Keypad9;
        case GlfwWindow::Key::KeypadDecimal: return ImGuiKey_KeypadDecimal;
        case GlfwWindow::Key::KeypadDivide: return ImGuiKey_KeypadDivide;
        case GlfwWindow::Key::KeypadMultiply: return ImGuiKey_KeypadMultiply;
        case GlfwWindow::Key::KeypadSubtract: return ImGuiKey_KeypadSubtract;
        case GlfwWindow::Key::KeypadAdd: return ImGuiKey_KeypadAdd;
        case GlfwWindow::Key::KeypadEnter: return ImGuiKey_KeypadEnter;
        case GlfwWindow::Key::KeypadEqual: return ImGuiKey_KeypadEqual;
        case GlfwWindow::Key::LeftShift: return ImGuiKey_LeftShift;
        case GlfwWindow::Key::LeftCtrl: return ImGuiKey_LeftCtrl;
        case GlfwWindow::Key::LeftAlt: return ImGuiKey_LeftAlt;
        case GlfwWindow::Key::LeftSuper: return ImGuiKey_LeftSuper;
        case GlfwWindow::Key::RightShift: return ImGuiKey_RightShift;
        case GlfwWindow::Key::RightCtrl: return ImGuiKey_RightCtrl;
        case GlfwWindow::Key::RightAlt: return ImGuiKey_RightAlt;
        case GlfwWindow::Key::RightSuper: return ImGuiKey_RightSuper;
        case GlfwWindow::Key::Menu: return ImGuiKey_Menu;
        case GlfwWindow::Key::Zero: return ImGuiKey_0;
        case GlfwWindow::Key::One: return ImGuiKey_1;
        case GlfwWindow::Key::Two: return ImGuiKey_2;
        case GlfwWindow::Key::Three: return ImGuiKey_3;
        case GlfwWindow::Key::Four: return ImGuiKey_4;
        case GlfwWindow::Key::Five: return ImGuiKey_5;
        case GlfwWindow::Key::Six: return ImGuiKey_6;
        case GlfwWindow::Key::Seven: return ImGuiKey_7;
        case GlfwWindow::Key::Eight: return ImGuiKey_8;
        case GlfwWindow::Key::Nine: return ImGuiKey_9;
        case GlfwWindow::Key::A: return ImGuiKey_A;
        case GlfwWindow::Key::B: return ImGuiKey_B;
        case GlfwWindow::Key::C: return ImGuiKey_C;
        case GlfwWindow::Key::D: return ImGuiKey_D;
        case GlfwWindow::Key::E: return ImGuiKey_E;
        case GlfwWindow::Key::F: return ImGuiKey_F;
        case GlfwWindow::Key::G: return ImGuiKey_G;
        case GlfwWindow::Key::H: return ImGuiKey_H;
        case GlfwWindow::Key::I: return ImGuiKey_I;
        case GlfwWindow::Key::J: return ImGuiKey_J;
        case GlfwWindow::Key::K: return ImGuiKey_K;
        case GlfwWindow::Key::L: return ImGuiKey_L;
        case GlfwWindow::Key::M: return ImGuiKey_M;
        case GlfwWindow::Key::N: return ImGuiKey_N;
        case GlfwWindow::Key::O: return ImGuiKey_O;
        case GlfwWindow::Key::P: return ImGuiKey_P;
        case GlfwWindow::Key::Q: return ImGuiKey_Q;
        case GlfwWindow::Key::R: return ImGuiKey_R;
        case GlfwWindow::Key::S: return ImGuiKey_S;
        case GlfwWindow::Key::T: return ImGuiKey_T;
        case GlfwWindow::Key::U: return ImGuiKey_U;
        case GlfwWindow::Key::V: return ImGuiKey_V;
        case GlfwWindow::Key::W: return ImGuiKey_W;
        case GlfwWindow::Key::X: return ImGuiKey_X;
        case GlfwWindow::Key::Y: return ImGuiKey_Y;
        case GlfwWindow::Key::Z: return ImGuiKey_Z;
        case GlfwWindow::Key::F1: return ImGuiKey_F1;
        case GlfwWindow::Key::F2: return ImGuiKey_F2;
        case GlfwWindow::Key::F3: return ImGuiKey_F3;
        case GlfwWindow::Key::F4: return ImGuiKey_F4;
        case GlfwWindow::Key::F5: return ImGuiKey_F5;
        case GlfwWindow::Key::F6: return ImGuiKey_F6;
        case GlfwWindow::Key::F7: return ImGuiKey_F7;
        case GlfwWindow::Key::F8: return ImGuiKey_F8;
        case GlfwWindow::Key::F9: return ImGuiKey_F9;
        case GlfwWindow::Key::F10: return ImGuiKey_F10;
        case GlfwWindow::Key::F11: return ImGuiKey_F11;
        case GlfwWindow::Key::F12: return ImGuiKey_F12;
        }

        assert(false);
        return ImGuiKey_None;
    }

    MouseCursorIcon translate(ImGuiMouseCursor cursor)
    {
        switch (cursor)
        {
            case ImGuiMouseCursor_Arrow: return MouseCursorIcon::Arrow;
            case ImGuiMouseCursor_TextInput: return MouseCursorIcon::TextInput;
            case ImGuiMouseCursor_ResizeAll: return MouseCursorIcon::ResizeAll;
            case ImGuiMouseCursor_ResizeNS: return MouseCursorIcon::ResizeNS;
            case ImGuiMouseCursor_ResizeEW: return MouseCursorIcon::ResizeEW;
            case ImGuiMouseCursor_ResizeNESW: return MouseCursorIcon::ResizeNESW;
            case ImGuiMouseCursor_ResizeNWSE: return MouseCursorIcon::ResizeNWSE;
            case ImGuiMouseCursor_NotAllowed: return MouseCursorIcon::NotAllowed;
            case ImGuiMouseCursor_Hand: return MouseCursorIcon::Hand;
        }

        assert(false);
        return MouseCursorIcon::Arrow;
    }

    void updateModifiers(GlfwWindow::Modifiers modifiers)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddKeyEvent(ImGuiKey_ModCtrl, (modifiers & GlfwWindow::Modifiers::Ctrl) != 0);
        io.AddKeyEvent(ImGuiKey_ModShift, (modifiers & GlfwWindow::Modifiers::Shift) != 0);
        io.AddKeyEvent(ImGuiKey_ModAlt, (modifiers & GlfwWindow::Modifiers::Alt) != 0);
    }
}

ImGuiPlatform::ImGuiPlatform(GlfwWindow& window) : m_window(window)
{
    static auto scopeId = memory::tracking::create_scope_id("UI/ImGui/Platform");
    MEMORY_TRACKING_SCOPE(scopeId);

    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = "vulkan_renderer_demo";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

    io.SetClipboardTextFn = [](void* userData, char const* text) { return static_cast<GlfwWindow*>(userData)->setClipboardText(text); };
    io.GetClipboardTextFn = [](void* userData) { return static_cast<GlfwWindow*>(userData)->getClipboardText(); };
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
    m_window.addFocusCallback([](bool focused) {
        ImGui::GetIO().AddFocusEvent(focused);
    });

    m_window.addMouseButtonCallback([](GlfwWindow::Action action, GlfwWindow::MouseButton button, GlfwWindow::Modifiers modifiers) {
        if (action != GlfwWindow::Action::Press && action != GlfwWindow::Action::Release)
            return;

        updateModifiers(modifiers);
        ImGui::GetIO().AddMouseButtonEvent(translate(button), action == GlfwWindow::Action::Press);
    });

    m_window.addCursorPositionCallback([](float x, float y) {
        ImGui::GetIO().AddMousePosEvent(x, y);
    });

    m_window.addKeyCallback([](GlfwWindow::Action action, GlfwWindow::Key key, GlfwWindow::Modifiers modifiers) {
        if (action != GlfwWindow::Action::Press && action != GlfwWindow::Action::Release)
            return;

        updateModifiers(modifiers);

//         keycode = ImGui_ImplGlfw_TranslateUntranslatedKey(keycode, scancode); // TODO

        ImGui::GetIO().AddKeyEvent(translate(key), action == GlfwWindow::Action::Press);
    });

    m_window.addCharCallback([](char c) {
        ImGui::GetIO().AddInputCharacter(c);
    });

    m_window.addScrollCallback([](float x, float y) {
        ImGui::GetIO().AddMouseWheelEvent(x, y);
    });
}

void ImGuiPlatform::updateDisplaySize()
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowSize = { 1.0f * m_window.getWindowWidth(), 1.0f * m_window.getWindowHeight() };
    ImVec2 framebufferSize = { 1.0f * m_window.getFramebufferWidth(), 1.0f * m_window.getFramebufferHeight() };
    io.DisplaySize = windowSize;
    if (windowSize.x > 0 && windowSize.y > 0)
        io.DisplayFramebufferScale = ImVec2(framebufferSize.x / windowSize.x, framebufferSize.y / windowSize.y);
}

void ImGuiPlatform::updateCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange))
        return;

    if (m_window.getCursorMode() == MouseCursorMode::Disabled)
        return;

    ImGuiMouseCursor imguiCursor = ImGui::GetMouseCursor();
    if (imguiCursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
    {
        m_window.setCursorMode(MouseCursorMode::Hidden);
    }
    else
    {
        m_window.setCursorMode(MouseCursorMode::Normal);
        m_window.setCursorIcon(translate(imguiCursor));
    }
}
