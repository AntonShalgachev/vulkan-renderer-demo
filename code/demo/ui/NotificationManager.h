#pragma once

#include <deque>
#include <string>

#include "services/ServiceContainer.h"

#include "ScopedDebugCommands.h"

namespace ui
{
    struct Notification
    {
        std::string text;
        float remainingTime = 0.0f;
    };

    class NotificationManager : public ServiceContainer
    {
    public:
        NotificationManager(Services& services);

        void update(float dt);
        void draw() const;

        void add(std::string text);

    private:
        ScopedDebugCommands m_commands{ services() };
        std::deque<Notification> m_notifications;
    };
}