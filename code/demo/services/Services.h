#pragma once

#include <memory>

class CommandLineService;
class DebugConsoleService;

class Services
{
public:
    ~Services();

    CommandLineService& commandLine() { assert(m_commandLine); return *m_commandLine; }
    void setCommandLine(std::unique_ptr<CommandLineService> service);

    DebugConsoleService& debugConsole() { assert(m_debugConsole); return *m_debugConsole; }
    void setDebugConsole(std::unique_ptr<DebugConsoleService> service);

    std::unique_ptr<CommandLineService> m_commandLine;
    std::unique_ptr<DebugConsoleService> m_debugConsole;
};
