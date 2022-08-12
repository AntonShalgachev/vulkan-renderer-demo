#include "Services.h"

#include "CommandLineService.h"
#include "DebugConsoleService.h"

Services::~Services() = default;

void Services::setCommandLine(std::unique_ptr<CommandLineService> service)
{
    m_commandLine = std::move(service);
}

void Services::setDebugConsole(std::unique_ptr<DebugConsoleService> service)
{
    m_debugConsole = std::move(service);
}
