#include "ScopedDebugCommands.h"

ScopedDebugCommands::ScopedDebugCommands(Services& services) : ServiceContainer(services)
{

}

ScopedDebugCommands::ScopedDebugCommands(ScopedDebugCommands&& rhs) : ServiceContainer(rhs.services())
{
    clear();

    m_names = nstl::move(rhs.m_names);
}

ScopedDebugCommands::~ScopedDebugCommands()
{
    clear();
}

ScopedDebugCommands& ScopedDebugCommands::operator=(ScopedDebugCommands&& rhs)
{
    clear();

    m_names = nstl::move(rhs.m_names);
    return *this;
}

void ScopedDebugCommands::add(nstl::string_view name, CommandMetadata metadata, coil::AnyFunctor anyFunctor)
{
    coil::Vector<coil::AnyFunctor> functors;
    functors.pushBack(nstl::move(anyFunctor));
    return add(name, nstl::move(metadata), nstl::move(functors));
}

void ScopedDebugCommands::add(nstl::string_view name, CommandMetadata metadata, coil::Vector<coil::AnyFunctor> anyFunctors)
{
    services().debugConsole().add(name, nstl::move(metadata), nstl::move(anyFunctors));
    m_names.push_back(name);
}

void ScopedDebugCommands::remove(nstl::string_view name)
{
    services().debugConsole().remove(name);
    m_names.erase_unsorted(name);
}

CommandProxy<ScopedDebugCommands> ScopedDebugCommands::operator[](nstl::string_view name)
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
