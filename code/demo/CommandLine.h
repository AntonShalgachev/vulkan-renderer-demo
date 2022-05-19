#pragma once

#include "argparse/argparse.hpp"
#include "ScopedDebugCommands.h"

class CommandLine
{
public:
    static CommandLine& instance(); // TODO remove

    CommandLine();

    template <typename... Targs>
    argparse::Argument& add(Targs... args)
    {
        return m_parser.add_argument(std::move(args)...);
    }

    bool parse(int argc, char** argv);
    bool parse(std::vector<std::string> const& arguments);
    bool parseFile(char const* path);

    template <typename T = std::string>
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
    ScopedDebugCommands m_commands;
    argparse::ArgumentParser m_parser;
    std::vector<std::string> m_arguments;
};
