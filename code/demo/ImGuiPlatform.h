#pragma once

namespace platform
{
    class window;
}

class ImGuiPlatform
{
public:
    ImGuiPlatform(platform::window& window);
    ~ImGuiPlatform();

    void update(float frameTime);

private:
    void setupCallbacks();

    void updateDisplaySize();
    void updateCursor();

private:
    platform::window& m_window;
};
