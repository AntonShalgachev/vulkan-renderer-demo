#include "MemoryViewerWindow.h"

#include "nstl/string_view.h"
#include "nstl/span.h"

#include "memory/tracking.h"

#include "common/charming_enum.h"

#include "imgui.h"

template <>
struct charming_enum::customize::enum_range<ui::MemoryViewerWindow::SizeUnit> {
    static constexpr int min = 0;
    static constexpr int max = 10;
};

ui::MemoryViewerWindow::MemoryViewerWindow(Services& services) : ServiceContainer(services)
{

}

void ui::MemoryViewerWindow::draw()
{
    ImGui::Begin("Memory Viewer");

    auto unitPrettyName = [](SizeUnit unit) -> char const*
    {
        switch (unit)
        {
        case SizeUnit::Bytes:
            return "Bytes";
        case SizeUnit::Kilobytes:
            return "Kilobytes";
        case SizeUnit::Megabytes:
            return "Megabytes";
        }
    };

    if (ImGui::BeginCombo("Size units", unitPrettyName(m_sizeUnits)))
    {
        for (SizeUnit unit : charming_enum::enum_values<SizeUnit>())
        {
            bool isSelected = m_sizeUnits == unit;
            if (ImGui::Selectable(unitPrettyName(unit), isSelected))
                m_sizeUnits = unit;

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

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

        char const* suffix = nullptr;
        size_t sizeDenominator = 0;

        switch (m_sizeUnits)
        {
        case SizeUnit::Bytes:
            suffix = "";
            sizeDenominator = 1;
            break;
        case SizeUnit::Kilobytes:
            suffix = " KB";
            sizeDenominator = 1024;
            break;
        case SizeUnit::Megabytes:
            suffix = " MB";
            sizeDenominator = 1024 * 1024;
            break;
        default:
            assert(false);
            break;
        }

        auto addSizeText = [suffix, sizeDenominator](size_t size) {
            if (sizeDenominator == 1)
                ImGui::Text("%zu%s", size, suffix);
            else
                ImGui::Text("%.2f%s", 1.0f * size / sizeDenominator, suffix);
        };

        ImGui::TableNextColumn();
        addSizeText(entry.active_bytes);

        ImGui::TableNextColumn();
        addSizeText(entry.total_bytes);

        ImGui::TableNextColumn();
        ImGui::Text("%zu", entry.active_allocations);
        ImGui::TableNextColumn();
        ImGui::Text("%zu", entry.total_allocations);
    }

    ImGui::EndTable();
}
