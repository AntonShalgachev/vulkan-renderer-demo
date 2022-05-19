#pragma once

#include <deque>
#include <string>

#include "../ScopedDebugCommands.h"

namespace ui
{
    struct Notification
    {
        std::string text;
        float remainingTime = 0.0f;
    };

    class NotificationManager
    {
    public:
        NotificationManager();

        void update(float dt);
        void draw() const;

        void add(std::string text);

    private:
        ScopedDebugCommands m_commands;
        std::deque<Notification> m_notifications;
    };
}