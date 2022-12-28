#pragma once

class GlfwWindow;

class ImGuiPlatform
{
public:
    ImGuiPlatform(GlfwWindow& window);
    ~ImGuiPlatform();

    void update(float frameTime);

private:
    void setupCallbacks();

    void updateDisplaySize();
    void updateCursor();

private:
    GlfwWindow& m_window;
};
