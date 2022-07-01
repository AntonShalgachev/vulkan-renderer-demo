#pragma once

#include <vector>
#include <string_view>

#include "DebugConsole.h"

class ScopedDebugCommands
{
public:
    ScopedDebugCommands();
    ScopedDebugCommands(ScopedDebugCommands&& rhs);

    ~ScopedDebugCommands();

    ScopedDebugCommands& operator=(ScopedDebugCommands&& rhs);

    template<typename Functor>
    void add(std::string_view name, Functor&& functor)
    {
        DebugConsole::instance().bindings().add(name, std::forward<Functor>(functor));
        m_names.push_back(name);
    }

    coil::BindingProxy<ScopedDebugCommands> operator[](std::string_view name);

private:
    void clear();

    std::vector<std::string_view> m_names; // TODO should be std::string
};