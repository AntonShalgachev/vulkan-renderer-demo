#pragma once

#include <memory>

#include <assert.h>

class CommandLineService;
class DebugConsoleService;
class DebugDrawService;

class Services
{
public:
    ~Services();

    Services& operator=(Services&& rhs) = default;

    CommandLineService& commandLine() { assert(m_commandLine); return *m_commandLine; }
    void setCommandLine(std::unique_ptr<CommandLineService> service);

    DebugConsoleService& debugConsole() { assert(m_debugConsole); return *m_debugConsole; }
    void setDebugConsole(std::unique_ptr<DebugConsoleService> service);

    DebugDrawService& debugDraw() { assert(m_debugDraw); return *m_debugDraw; }
    void setDebugDraw(std::unique_ptr<DebugDrawService> service);

    std::unique_ptr<CommandLineService> m_commandLine;
    std::unique_ptr<DebugConsoleService> m_debugConsole;
    std::unique_ptr<DebugDrawService> m_debugDraw;
};
