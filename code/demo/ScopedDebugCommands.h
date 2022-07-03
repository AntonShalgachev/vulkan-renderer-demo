#pragma once

#include "DebugConsole.h"
#include "CommandMetadata.h"
#include "CommandProxy.h"

#include <vector>
#include <string_view>

class ScopedDebugCommands
{
public:
    ScopedDebugCommands();
    ScopedDebugCommands(ScopedDebugCommands&& rhs);

    ~ScopedDebugCommands();

    ScopedDebugCommands& operator=(ScopedDebugCommands&& rhs);

    template<typename Functor>
    void add(std::string_view name, CommandMetadata metadata, Functor&& functor)
    {
        DebugConsole::instance().add(name, std::move(metadata), std::forward<Functor>(functor));
        m_names.push_back(name);
    }

    void remove(std::string_view name);

    CommandProxy<ScopedDebugCommands> operator[](std::string_view name);

private:
    void clear();

    std::vector<std::string_view> m_names; // TODO should be std::string
};
