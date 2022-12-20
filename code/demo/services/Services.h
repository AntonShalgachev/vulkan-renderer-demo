#pragma once

#include "nstl/unique_ptr.h"

#include <assert.h>

class CommandLineService;
class DebugConsoleService;
class DebugDrawService;

class Services
{
public:
    Services();
    ~Services();

    Services& operator=(Services&& rhs) = default;

    CommandLineService& commandLine() { assert(m_commandLine); return *m_commandLine; }
    void setCommandLine(nstl::unique_ptr<CommandLineService> service);

    DebugConsoleService& debugConsole() { assert(m_debugConsole); return *m_debugConsole; }
    void setDebugConsole(nstl::unique_ptr<DebugConsoleService> service);

    DebugDrawService& debugDraw() { assert(m_debugDraw); return *m_debugDraw; }
    void setDebugDraw(nstl::unique_ptr<DebugDrawService> service);

    nstl::unique_ptr<CommandLineService> m_commandLine;
    nstl::unique_ptr<DebugConsoleService> m_debugConsole;
    nstl::unique_ptr<DebugDrawService> m_debugDraw;
};
