#include "CommandLineService.h"
#include "DebugConsoleService.h"

#include "spdlog/spdlog.h"

#include <fstream>

namespace
{
    void append(nstl::string_view line, nstl::vector<nstl::string>& arguments)
    {
        auto isQuote = [](char c) { return c == '"'; };
        auto isComment = [](char c) { return c == ';' || c == '#'; };
        auto isChar = [&isComment, &isQuote](char c) { return !std::isspace(c) && !isComment(c) && !isQuote(c); };

        for (std::size_t i = 0; i < line.size(); i++)
        {
            while ((i < line.size()) && std::isspace(line[i]))
                i++;

            if (i >= line.size())
                break;

            if (isComment(line[i]))
                break;

            if (isQuote(line[i]))
            {
                i++;
                std::size_t tokenBegin = i;
                while ((i < line.size()) && !isQuote(line[i]))
                    i++;
                arguments.emplace_back(line.substr(tokenBegin, i - tokenBegin));
            }
            else if (isChar(line[i]))
            {
                std::size_t tokenBegin = i;
                i++;
                while ((i < line.size()) && isChar(line[i]))
                    i++;
                arguments.emplace_back(line.substr(tokenBegin, i - tokenBegin));
                i--;
            }
        }
    }
}

CommandLineService::CommandLineService(Services& services) : ServiceContainer(services)
{
    m_commands["cmdline.list"] = [this](coil::Context context)
    {
        for (auto const& arg : m_arguments)
            context.loglinef("%s", arg.c_str());
    };
}

bool CommandLineService::parse(int argc, char** argv)
{
    nstl::vector<nstl::string> arguments;
    arguments.reserve(argc);
    for (size_t i = 0; i < argc; i++)
        arguments.emplace_back(argv[i]);
    return parse(arguments);
}

bool CommandLineService::parse(nstl::vector<nstl::string> const& arguments)
{
    try
    {
        // TODO fix this ugliness
        std::vector<std::string> stdArguments;
        for (nstl::string const& arg : arguments)
            stdArguments.emplace_back(arg.c_str());
        m_parser.parse_args(stdArguments);

        if (!arguments.empty())
        {
            for (auto it = std::next(arguments.begin()); it != arguments.end(); it++)
                m_arguments.push_back(*it);
        }

        return true;
    }
    catch (const std::runtime_error& err)
    {
        std::cerr << err.what() << std::endl;
    }

    return false;
}

bool CommandLineService::parseFile(char const* path)
{
    std::ifstream file{ path };

    if (!file)
        return false;

    nstl::vector<nstl::string> arguments;
    arguments.push_back(""); // fake program name
    for (std::string line; std::getline(file, line); )
    {
        // TODO fix this ugliness
        append(nstl::string_view{ line.data(), line.size() }, arguments);
    }

    return parse(arguments);
}
