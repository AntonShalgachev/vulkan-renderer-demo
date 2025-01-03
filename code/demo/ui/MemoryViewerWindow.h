#pragma once

#include "services/ServiceContainer.h"

#include "common/tiny_ctti.h"

#include "nstl/string_view.h"
#include "nstl/vector.h"
#include "nstl/unordered_map.h"

namespace ui
{
    class MemoryViewerWindow : public ServiceContainer
    {
    public: // TODO make private or move to cpp
        enum class SizeUnit
        {
            Bytes,
            Kilobytes,
            Megabytes,
        };
        TINY_CTTI_DESCRIBE_NESTED_ENUM(SizeUnit, Bytes, Kilobytes, Megabytes);

        struct TreeNode
        {
            nstl::string_view path;
            nstl::string_view name;

            nstl::vector<size_t> childrenIndices;

            size_t activeBytes = 0;
            size_t totalBytes = 0;
            size_t activeAllocations = 0;
            size_t totalAllocations = 0;

            void reset()
            {
                activeBytes = 0;
                totalBytes = 0;
                activeAllocations = 0;
                totalAllocations = 0;
            }
        };

    public:
        MemoryViewerWindow(Services& services);

        void draw();
        void toggle() { m_opened = !m_opened; }

    private:
        void drawTable();
        void drawTreeNode(size_t index);

    private:
        bool m_opened = false;
        SizeUnit m_sizeUnits = SizeUnit::Bytes;

        nstl::vector<TreeNode> m_nodes;
        nstl::unordered_map<nstl::string_view, size_t> m_nameToNodeIndexMap;
    };
}
