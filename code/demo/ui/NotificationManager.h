#pragma once

#include "services/ServiceContainer.h"

#include "ScopedDebugCommands.h"

#include "nstl/string.h"
#include "nstl/vector.h"

namespace ui
{
    struct Notification
    {
        nstl::string text;
        float remainingTime = 0.0f;
    };

    class NotificationManager : public ServiceContainer
    {
    public:
        NotificationManager(Services& services);

        void update(float dt);
        void draw() const;

        void add(nstl::string text);

    private:
        ScopedDebugCommands m_commands{ services() };
        nstl::vector<Notification> m_notifications;
    };
}