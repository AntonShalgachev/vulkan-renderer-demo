#include "ScopedDebugCommands.h"

#include "memory/tracking.h"

namespace
{
    auto scopeId = memory::tracking::create_scope_id("System/ScopedCommands");
}

ScopedDebugCommands::ScopedDebugCommands(Services& services) : ServiceContainer(services)
{
    MEMORY_TRACKING_SCOPE(scopeId);
}

ScopedDebugCommands::ScopedDebugCommands(ScopedDebugCommands&& rhs) : ServiceContainer(rhs.services())
{
    MEMORY_TRACKING_SCOPE(scopeId);

    clear();

    m_names = nstl::move(rhs.m_names);
}

ScopedDebugCommands::~ScopedDebugCommands()
{
    MEMORY_TRACKING_SCOPE(scopeId);

    clear();
}

ScopedDebugCommands& ScopedDebugCommands::operator=(ScopedDebugCommands&& rhs)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    clear();

    m_names = nstl::move(rhs.m_names);
    return *this;
}

void ScopedDebugCommands::add(nstl::string_view name, CommandMetadata metadata, coil::AnyFunctor anyFunctor)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    coil::Vector<coil::AnyFunctor> functors;
    functors.pushBack(nstl::move(anyFunctor));
    return add(name, nstl::move(metadata), nstl::move(functors));
}

void ScopedDebugCommands::add(nstl::string_view name, CommandMetadata metadata, coil::Vector<coil::AnyFunctor> anyFunctors)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    services().debugConsole().add(name, nstl::move(metadata), nstl::move(anyFunctors));
    m_names.push_back(name);
}

void ScopedDebugCommands::remove(nstl::string_view name)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    services().debugConsole().remove(name);
    m_names.erase_unsorted(name);
}

CommandProxy<ScopedDebugCommands> ScopedDebugCommands::operator[](nstl::string_view name)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return { *this, name };
}

void ScopedDebugCommands::clear()
{
    MEMORY_TRACKING_SCOPE(scopeId);

    for (auto const& name : m_names)
        services().debugConsole().remove(name);

    m_names.clear();
}

template class CommandProxy<ScopedDebugCommands>;
