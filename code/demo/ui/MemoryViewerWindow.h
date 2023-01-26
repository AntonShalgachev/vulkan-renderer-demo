#pragma once

#include "services/ServiceContainer.h"

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

    private:
        void drawTable();
        void drawTreeNode(size_t index);

    private:
        SizeUnit m_sizeUnits = SizeUnit::Bytes;

        nstl::vector<TreeNode> m_nodes;
        nstl::unordered_map<nstl::string_view, size_t> m_nameToNodeIndexMap;
    };
}
