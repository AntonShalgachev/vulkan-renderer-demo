#pragma once

#include "services/ServiceContainer.h"

namespace ui
{
    class MemoryViewerWindow : public ServiceContainer
    {
    public:
        MemoryViewerWindow(Services& services);

        void draw();

    private:
        enum class SizeUnit
        {
            Bytes,
            Kilobytes,
            Megabytes,
        };

    private:
        void drawTable();

    private:
        SizeUnit m_sizeUnits = SizeUnit::Bytes;
    };
}
