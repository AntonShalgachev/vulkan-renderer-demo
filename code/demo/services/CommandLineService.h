#pragma once

#include "ServiceContainer.h"
#include "ScopedDebugCommands.h"

#include "nstl/vector.h"
#include "nstl/string.h"
#include "nstl/unordered_map.h"

class CommandLineService : public ServiceContainer
{
public:
    CommandLineService(Services& services);

    void add(int argc, char** argv);
    void addLine(nstl::string_view line);
    void add(nstl::string arg);

    bool parse();

    nstl::vector<nstl::string> const& getAll() const { return m_arguments; }

    nstl::span<nstl::string_view const> get(nstl::string_view name) const;

private:
    ScopedDebugCommands m_commands{ services() };
    nstl::vector<nstl::string> m_arguments;
    nstl::unordered_map<nstl::string_view, nstl::vector<nstl::string_view>> m_argMap;
};
