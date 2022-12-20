#include "Services.h"

#include "CommandLineService.h"
#include "DebugConsoleService.h"
#include "DebugDrawService.h"

Services::Services() = default;
Services::~Services() = default;

void Services::setCommandLine(nstl::unique_ptr<CommandLineService> service)
{
    m_commandLine = std::move(service);
}

void Services::setDebugConsole(nstl::unique_ptr<DebugConsoleService> service)
{
    m_debugConsole = std::move(service);
}

void Services::setDebugDraw(nstl::unique_ptr<DebugDrawService> service)
{
    m_debugDraw = std::move(service);
}
