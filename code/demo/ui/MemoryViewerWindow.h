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
        void drawTable();

    private:
        bool m_convertToMb = false;
    };
}
