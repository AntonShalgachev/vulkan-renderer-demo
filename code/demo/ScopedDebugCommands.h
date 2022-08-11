#pragma once

#include "CommandMetadata.h"
#include "CommandProxy.h"

#include "services/ServiceContainer.h"
#include "services/Services.h"
#include "services/DebugConsoleService.h"

#include <vector>
#include <string_view>

class ScopedDebugCommands : public ServiceContainer
{
public:
    ScopedDebugCommands(Services& services);
    ScopedDebugCommands(ScopedDebugCommands&& rhs);

    ~ScopedDebugCommands();

    ScopedDebugCommands& operator=(ScopedDebugCommands&& rhs);

    template<typename Functor>
    void add(std::string_view name, CommandMetadata metadata, Functor&& functor)
    {
        services().debugConsole().add(name, std::move(metadata), std::forward<Functor>(functor));
        m_names.push_back(name);
    }

    void remove(std::string_view name);
    void clear();

    CommandProxy<ScopedDebugCommands> operator[](std::string_view name);

private:
    std::vector<std::string_view> m_names; // TODO should be std::string
};
