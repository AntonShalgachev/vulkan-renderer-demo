#include "CommandLine.h"
#include "DebugConsole.h"

#include <fstream>

namespace
{
    void append(std::string_view line, std::vector<std::string>& arguments)
    {
        auto isQuote = [](char c) { return c == '"'; };
        auto isComment = [](char c) { return c == ';'; };
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

CommandLine& CommandLine::instance()
{
    static CommandLine instance;
    return instance;
}

CommandLine::CommandLine()
{
    m_commands["cmdline.list"] = [this](coil::Context context)
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
        m_parser.parse_args(arguments);

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

bool CommandLine::parseFile(char const* path)
{
    std::ifstream file{ path };

    if (!file)
        return false;

    std::vector<std::string> arguments;
    arguments.push_back(""); // fake program name
    for (std::string line; std::getline(file, line); )
    {
        append(line, arguments);
    }

    return parse(arguments);
}
