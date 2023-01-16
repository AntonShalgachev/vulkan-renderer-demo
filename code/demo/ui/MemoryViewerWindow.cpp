#include "MemoryViewerWindow.h"

#include "nstl/string_view.h"
#include "nstl/span.h"

#include "memory/tracking.h"

#include "imgui.h"

ui::MemoryViewerWindow::MemoryViewerWindow(Services& services) : ServiceContainer(services)
{

}

void ui::MemoryViewerWindow::draw()
{
    ImGui::Begin("Memory Viewer");

    ImGui::Checkbox("Convert to MB", &m_convertToMb);

    drawTable();

    ImGui::End();
}

void ui::MemoryViewerWindow::drawTable()
{
    ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

    if (!ImGui::BeginTable("allocations", 5, flags))
        return;

    ImGui::TableSetupColumn("Scope");
    ImGui::TableSetupColumn("Bytes");
    ImGui::TableSetupColumn("Bytes ever");
    ImGui::TableSetupColumn("Allocations");
    ImGui::TableSetupColumn("Allocations ever");
    ImGui::TableHeadersRow();

    nstl::span<memory::tracking::scope_stat const> entries = memory::tracking::get_scope_stats();

    for (size_t i = 0; i < entries.size(); i++)
    {
        memory::tracking::scope_stat const& entry = entries[i];

        nstl::string_view scopeName = memory::tracking::get_scope_name(entry.id);
        if (scopeName.empty())
            scopeName = "Root";

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        // TODO make use of trees

        ImGui::PushID(i);
        ImGui::TreeNodeEx("", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth, "%.*s", scopeName.slength(), scopeName.data());
        ImGui::PopID();

        ImGui::TableNextColumn();
        if (m_convertToMb)
            ImGui::Text("%.2f MB", 1.0f * entry.active_bytes / 1024 / 1024);
        else
            ImGui::Text("%zu", entry.active_bytes);

        ImGui::TableNextColumn();
        if (m_convertToMb)
            ImGui::Text("%.2f MB", 1.0f * entry.total_bytes / 1024 / 1024);
        else
            ImGui::Text("%zu", entry.total_bytes);

        ImGui::TableNextColumn();
        ImGui::Text("%zu", entry.active_allocations);
        ImGui::TableNextColumn();
        ImGui::Text("%zu", entry.total_allocations);
    }

    ImGui::EndTable();
}
