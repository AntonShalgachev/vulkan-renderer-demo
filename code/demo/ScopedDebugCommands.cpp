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

void ScopedDebugCommands::add(std::string_view name, CommandMetadata metadata, coil::AnyFunctor anyFunctor)
{
    coil::Vector<coil::AnyFunctor> functors;
    functors.pushBack(std::move(anyFunctor));
    return add(name, std::move(metadata), std::move(functors));
}

void ScopedDebugCommands::add(std::string_view name, CommandMetadata metadata, coil::Vector<coil::AnyFunctor> anyFunctors)
{
    services().debugConsole().add(name, std::move(metadata), std::move(anyFunctors));
    m_names.push_back(name);
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

template class CommandProxy<ScopedDebugCommands>;
