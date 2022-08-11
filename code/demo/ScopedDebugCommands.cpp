#include "ScopedDebugCommands.h"

ScopedDebugCommands::ScopedDebugCommands(Services& services) : ServiceContainer(services)
{

}

ScopedDebugCommands::ScopedDebugCommands(ScopedDebugCommands&& rhs) : ServiceContainer(rhs.services())
{
    clear();

    m_names = std::move(rhs.m_names);
}

ScopedDebugCommands::~ScopedDebugCommands()
{
    clear();
}

ScopedDebugCommands& ScopedDebugCommands::operator=(ScopedDebugCommands&& rhs)
{
    clear();

    m_names = std::move(rhs.m_names);
    return *this;
}

void ScopedDebugCommands::remove(std::string_view name)
{
    services().debugConsole().remove(name);
    m_names.erase(std::remove(m_names.begin(), m_names.end(), name), m_names.end());
}

CommandProxy<ScopedDebugCommands> ScopedDebugCommands::operator[](std::string_view name)
{
    return { *this, name };
}

void ScopedDebugCommands::clear()
{
    for (auto const& name : m_names)
        services().debugConsole().remove(name);

    m_names.clear();
}
