#include "CommandLineService.h"
#include "DebugConsoleService.h"

#include <ctype.h>

// TODO rework this class

namespace
{
    void append(nstl::string_view line, nstl::vector<nstl::string>& arguments)
    {
        auto isQuote = [](char c) { return c == '"' || c == '\''; };
        auto isComment = [](char c) { return c == ';' || c == '#'; };
        auto isChar = [&isComment, &isQuote](char c) { return !isspace(c) && !isComment(c) && !isQuote(c); };

        for (size_t i = 0; i < line.size(); i++)
        {
            while ((i < line.size()) && isspace(line[i]))
                i++;

            if (i >= line.size())
                break;

            if (isComment(line[i]))
                break;

            if (isQuote(line[i]))
            {
                char openingQuote = line[i];

                i++;
                size_t tokenBegin = i;
                while ((i < line.size()) && line[i] != openingQuote)
                    i++;
                arguments.emplace_back(line.substr(tokenBegin, i - tokenBegin));
            }
            else if (isChar(line[i]))
            {
                size_t tokenBegin = i;
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

void CommandLineService::add(int argc, char** argv)
{
    m_arguments.reserve(m_arguments.size() + argc);
    for (size_t i = 0; i < argc; i++)
        add(argv[i]);
}

void CommandLineService::addLine(nstl::string_view line)
{
    append(line, m_arguments);
}

void CommandLineService::add(nstl::string arg)
{
    assert(m_argMap.empty()); // adding arguments might invalidate existing ones
    m_arguments.push_back(nstl::move(arg));
}

bool CommandLineService::parse()
{
    assert(!m_arguments.empty());
    assert(m_arguments.size() % 2 == 1);
    for (size_t i = 1; i < m_arguments.size(); i += 2)
    {
        nstl::string_view key = m_arguments[i];
        nstl::string_view value = m_arguments[i + 1];

        auto it = m_argMap.find(key);
        if (it != m_argMap.end())
            it->value().push_back(value);
        else
            m_argMap.insert_or_assign(key, { value });
    }

    return true;
}

nstl::span<nstl::string_view const> CommandLineService::get(nstl::string_view name) const
{
    auto it = m_argMap.find(name);
    if (it == m_argMap.end())
        return {};
    return it->value();
}
