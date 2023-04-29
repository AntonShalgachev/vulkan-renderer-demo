#include "ScopedDebugCommands.h"

#include "memory/tracking.h"

namespace
{
    auto scopedCommandsScopeId = memory::tracking::create_scope_id("System/ScopedCommands");
}

ScopedDebugCommands::ScopedDebugCommands(Services& services) : ServiceContainer(services)
{
    MEMORY_TRACKING_SCOPE(scopedCommandsScopeId);
}

ScopedDebugCommands::ScopedDebugCommands(ScopedDebugCommands&& rhs) : ServiceContainer(rhs.services())
{
    MEMORY_TRACKING_SCOPE(scopedCommandsScopeId);

    clear();

    m_names = nstl::move(rhs.m_names);
}

ScopedDebugCommands::~ScopedDebugCommands()
{
    MEMORY_TRACKING_SCOPE(scopedCommandsScopeId);

    clear();
}

ScopedDebugCommands& ScopedDebugCommands::operator=(ScopedDebugCommands&& rhs)
{
    MEMORY_TRACKING_SCOPE(scopedCommandsScopeId);

    clear();

    m_names = nstl::move(rhs.m_names);
    return *this;
}

void ScopedDebugCommands::add(nstl::string_view name, CommandMetadata metadata, coil::AnyFunctor anyFunctor)
{
    MEMORY_TRACKING_SCOPE(scopedCommandsScopeId);

    coil::Vector<coil::AnyFunctor> functors;
    functors.pushBack(nstl::move(anyFunctor));
    return add(name, nstl::move(metadata), nstl::move(functors));
}

void ScopedDebugCommands::add(nstl::string_view name, CommandMetadata metadata, coil::Vector<coil::AnyFunctor> anyFunctors)
{
    MEMORY_TRACKING_SCOPE(scopedCommandsScopeId);

    services().debugConsole().add(name, nstl::move(metadata), nstl::move(anyFunctors));
    m_names.push_back(name);
}

void ScopedDebugCommands::remove(nstl::string_view name)
{
    MEMORY_TRACKING_SCOPE(scopedCommandsScopeId);

    services().debugConsole().remove(name);
    m_names.erase_unsorted(name);
}

CommandProxy<ScopedDebugCommands> ScopedDebugCommands::operator[](nstl::string_view name)
{
    MEMORY_TRACKING_SCOPE(scopedCommandsScopeId);

    return { *this, name };
}

void ScopedDebugCommands::clear()
{
    MEMORY_TRACKING_SCOPE(scopedCommandsScopeId);

    for (auto const& name : m_names)
        services().debugConsole().remove(name);

    m_names.clear();
}

template class CommandProxy<ScopedDebugCommands>;
