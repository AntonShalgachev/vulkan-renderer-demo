#include "MemoryViewerWindow.h"

#include "nstl/string_view.h"
#include "nstl/span.h"
#include "nstl/vector.h"
#include "nstl/unordered_map.h"

#include "memory/tracking.h"

#include "imgui.h"

namespace
{
    struct ScopePart
    {
        nstl::string_view path;
        nstl::string_view name;

        bool operator==(ScopePart const&) const = default;
    };

    class ScopeWalkerIterator
    {
    public:
        ScopeWalkerIterator() = default;
        ScopeWalkerIterator(nstl::string_view fullPath) : m_fullPath(fullPath)
        {
            m_first = true;
        }
        ScopeWalkerIterator(nstl::string_view fullPath, size_t offset)
        {
            if (offset >= fullPath.length())
                return;

            m_fullPath = fullPath;

            m_nextSeparatorPos = fullPath.find('/', offset);
            if (m_nextSeparatorPos == nstl::string_view::npos)
                m_nextSeparatorPos = fullPath.size();

            m_self.path = fullPath.substr(0, m_nextSeparatorPos);
            m_self.name = fullPath.substr(offset, m_nextSeparatorPos - offset);
        }

        ScopePart operator*()
        {
            return m_self;
        }

        bool operator==(ScopeWalkerIterator const&) const = default;

        ScopeWalkerIterator& operator++()
        {
            if (m_first)
                *this = ScopeWalkerIterator{ m_fullPath, 0 };
            else
                *this = ScopeWalkerIterator{ m_fullPath, m_nextSeparatorPos + 1 };

            return *this;
        }

    private:
        nstl::string_view m_fullPath;
        size_t m_nextSeparatorPos = 0;
        bool m_first = false;

        ScopePart m_self;
    };

    class ScopeWalker
    {
    public:
        ScopeWalker(nstl::string_view fullPath) : m_fullPath(fullPath) {}

        auto begin()
        {
            return ScopeWalkerIterator{ m_fullPath };
        }

        auto end()
        {
            return ScopeWalkerIterator{};
        }

    private:
        nstl::string_view m_fullPath;
    };
}

ui::MemoryViewerWindow::MemoryViewerWindow(Services& services) : ServiceContainer(services)
{

}

void ui::MemoryViewerWindow::draw()
{
    if (!m_opened)
        return;

    ImGui::Begin("Memory Viewer", &m_opened);

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

        assert(false);
        return "";
    };

    if (ImGui::BeginCombo("Size units", unitPrettyName(m_sizeUnits)))
    {
        for (SizeUnit unit : tiny_ctti::enum_values<SizeUnit>())
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

    nstl::vector<memory::tracking::scope_stat> entries = memory::tracking::get_scope_stats_copy();

    for (TreeNode& node : m_nodes)
        node.reset();

    for (memory::tracking::scope_stat const& stat : entries)
    {
        nstl::string_view scopePath = memory::tracking::get_scope_name(stat.id);

        size_t parentNodeIndex = static_cast<size_t>(-1);

        for (ScopePart scopePart : ScopeWalker{ scopePath })
        {
            auto it = m_nameToNodeIndexMap.find(scopePart.path);
            if (it == m_nameToNodeIndexMap.end())
            {
                it = m_nameToNodeIndexMap.insert_or_assign(scopePart.path, m_nodes.size());

                if (parentNodeIndex != static_cast<size_t>(-1))
                    m_nodes[parentNodeIndex].childrenIndices.push_back(m_nodes.size());

                m_nodes.push_back(TreeNode{ .path = scopePart.path, .name = scopePart.name });
            }

            size_t nodeIndex = it->value();
            TreeNode& node = m_nodes[nodeIndex];

            node.activeBytes += stat.active_bytes;
            node.totalBytes += stat.total_bytes;
            node.activeAllocations += stat.active_allocations;
            node.totalAllocations += stat.total_allocations;

            parentNodeIndex = nodeIndex;
        }
    }

    assert(m_nodes[0].path == "");
    drawTreeNode(0);

    ImGui::EndTable();
}

void ui::MemoryViewerWindow::drawTreeNode(size_t index)
{
    TreeNode const& self = m_nodes[index];

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;
    if (self.childrenIndices.empty())
        treeNodeFlags = treeNodeFlags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

    // TODO handle it in a better way. PushID crashes with empty string_view containing nullptr
    if (self.path.empty())
        ImGui::PushID("");
    else
        ImGui::PushID(self.path.begin(), self.path.end());
    bool open = ImGui::TreeNodeEx("", treeNodeFlags, "%.*s", self.name.slength(), self.name.data());
    ImGui::PopID();

    char const* unitName = nullptr;
    size_t sizeDenominator = 0;

    switch (m_sizeUnits)
    {
    case SizeUnit::Bytes:
        unitName = "B";
        sizeDenominator = 1;
        break;
    case SizeUnit::Kilobytes:
        unitName = "KB";
        sizeDenominator = 1024;
        break;
    case SizeUnit::Megabytes:
        unitName = "MB";
        sizeDenominator = 1024 * 1024;
        break;
    }

    auto addSizeText = [unitName, sizeDenominator](size_t size)
    {
        if (sizeDenominator == 1)
            ImGui::Text("%zu %s", size, unitName);
        else
            ImGui::Text("%.2f %s", 1.0f * size / sizeDenominator, unitName);
    };

    ImGui::TableNextColumn();
    addSizeText(self.activeBytes);

    ImGui::TableNextColumn();
    addSizeText(self.totalBytes);

    ImGui::TableNextColumn();
    ImGui::Text("%zu", self.activeAllocations);
    ImGui::TableNextColumn();
    ImGui::Text("%zu", self.totalAllocations);

    if (open)
    {
        for (size_t childIndex : self.childrenIndices)
            drawTreeNode(childIndex);

        ImGui::TreePop();
    }
}
