#include "NotificationManager.h"

#include "services/DebugConsoleService.h"

#include "nstl/algorithm.h"

#include "imgui.h"

namespace
{
    float defaultDuration = 5.0f;
}

ui::NotificationManager::NotificationManager(Services& services) : ServiceContainer(services)
{
    m_commands["ui.notifications.add"] = coil::bind(&NotificationManager::add, this);
}

void ui::NotificationManager::update(float dt)
{
    for (Notification& notification : m_notifications)
        notification.remainingTime -= dt;

    m_notifications.erase(nstl::remove_if(m_notifications.begin(), m_notifications.end(), [](Notification const& notification){
        return notification.remainingTime <= 0.0f;
    }), m_notifications.end());
}

void ui::NotificationManager::draw() const
{
    if (m_notifications.empty())
        return;

    {
        const float padding = 10.0f;
        ImVec2 workPos = ImGui::GetMainViewport()->WorkPos;
        ImVec2 windowPos{ workPos.x + padding, workPos.y + padding };
        ImGui::SetNextWindowPos(windowPos);
    }

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs;
    ImGui::Begin("Notifications", nullptr, windowFlags);
    
    ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;
    for (Notification const& notification : m_notifications)
    {
        ImGui::PushID(&notification);
        if (ImGui::BeginTable("table_padding", 1, tableFlags))
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(notification.text.c_str(), ImGui::GetContentRegionAvail().x);
        }

        ImGui::EndTable();
        ImGui::PopID();
    }

    ImGui::End();
}

void ui::NotificationManager::add(nstl::string text)
{
    m_notifications.push_back({ text, defaultDuration });
}
