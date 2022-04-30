#include "CommandLine.h"
#include "DebugConsole.h"

CommandLine& CommandLine::instance()
{
    static CommandLine instance;
    return instance;
}

CommandLine::CommandLine()
{
    coil::Bindings& bindings = DebugConsole::instance().bindings();

    bindings["cmdline.list"] = [this](coil::Context context)
    {
        for (auto const& arg : m_arguments)
            context.out() << arg << std::endl;
    };
}

bool CommandLine::parse(int argc, char** argv)
{
    std::vector<std::string> arguments;
    std::copy(argv, argv + argc, std::back_inserter(arguments));
    return parse(arguments);
}

bool CommandLine::parse(std::vector<std::string> const& arguments)
{
    try
    {
        for (auto const& arg : arguments)
            m_arguments.push_back(arg);

        m_parser.parse_args(arguments);
    }
    catch (const std::runtime_error& err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << m_parser;
        return false;
    }

    return true;
}
