#pragma once

#include <deque>
#include <string>

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
        std::deque<Notification> m_notifications;
    };
}