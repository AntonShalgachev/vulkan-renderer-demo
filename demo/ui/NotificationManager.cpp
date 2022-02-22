#include "NotificationManager.h"
#include "imgui.h"

namespace
{
    float defaultDuration = 5.0f;
}

void ui::NotificationManager::update(float dt)
{
    for (Notification& notification : m_notifications)
        notification.remainingTime -= dt;

    while (!m_notifications.empty() && m_notifications.front().remainingTime <= 0.0f)
        m_notifications.pop_front();
}

void ui::NotificationManager::draw() const
{
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

void ui::NotificationManager::add(std::string text)
{
    m_notifications.push_back({ text, defaultDuration });
}
