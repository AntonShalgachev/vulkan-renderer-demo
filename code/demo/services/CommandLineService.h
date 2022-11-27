#pragma once

#include "ServiceContainer.h"
#include "ScopedDebugCommands.h"

#include "nstl/vector.h"

#include "argparse/argparse.hpp"

class CommandLineService : public ServiceContainer
{
public:
    CommandLineService(Services& services);

    template <typename... Targs>
    argparse::Argument& add(Targs... args)
    {
        return m_parser.add_argument(std::move(args)...);
    }

    bool parse(int argc, char** argv);
    bool parse(nstl::vector<nstl::string> const& arguments);
    bool parseFile(char const* path);

    nstl::vector<nstl::string> const& getAll() const { return m_arguments; }

    template <typename T = nstl::string>
    T get(std::string_view arg) const
    {
        try
        {
            return m_parser.get<T>(arg);
        }
        catch (std::exception const& ex)
        {
            // TODO
        }

        return T{};
    }

private:
    ScopedDebugCommands m_commands{ services() };
    argparse::ArgumentParser m_parser;
    nstl::vector<nstl::string> m_arguments;
};
