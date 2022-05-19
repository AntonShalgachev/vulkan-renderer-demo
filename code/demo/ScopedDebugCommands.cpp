#include "ScopedDebugCommands.h"

ScopedDebugCommands::ScopedDebugCommands() = default;

ScopedDebugCommands::ScopedDebugCommands(ScopedDebugCommands&& rhs)
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

coil::BindingProxy<ScopedDebugCommands> ScopedDebugCommands::operator[](std::string_view name)
{
    return coil::BindingProxy<ScopedDebugCommands>{ *this, name };
}

void ScopedDebugCommands::clear()
{
    auto& bindings = DebugConsole::instance().bindings();
    for (auto const& name : m_names)
        bindings.remove(name);
}
