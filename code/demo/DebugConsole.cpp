#include "DebugConsole.h"

#include "rapidfuzz/fuzz.hpp"

#include <iomanip>

namespace
{
    class CommandsListIterator
    {
    public:
        CommandsListIterator() = default;
        CommandsListIterator(std::string_view commands)
        {
            if (commands.empty())
                return;

            std::size_t i = 0;
            while (i < commands.size() && commands[i] != ';')
                i++;

            m_self = commands.substr(0, i);
            if (i < commands.size())
                m_rest = commands.substr(i + 1, commands.size() - i);
        }

        std::string_view operator*()
        {
            assert(m_self);
            return *m_self;
        }

        bool operator==(CommandsListIterator const& rhs) const
        {
            return m_self == rhs.m_self && m_rest == rhs.m_rest;
        }

        CommandsListIterator& operator++()
        {
            *this = CommandsListIterator{m_rest};
            return *this;
        }

    private:
        std::optional<std::string_view> m_self;
        std::string_view m_rest;
    };

    class CommandsList
    {
    public:
        CommandsList(std::string_view commands) : m_commands(commands) {}

        auto begin()
        {
            return CommandsListIterator{ m_commands };
        }

        auto end()
        {
            return CommandsListIterator{};
        }

    private:
        std::string_view m_commands;
    };
}

DebugConsole& DebugConsole::instance()
{
    static DebugConsole console;
    return console;
}

DebugConsole::DebugConsole()
{
    auto& commands = *this;

    commands["list"].description("Lists available commands") = [this](coil::Context context)
    {
        // TODO accept also wildcard

        std::size_t maxAllowedNameLength = 50;

        std::size_t maxNameLength = 0;
        for (auto const& command : m_bindings.commands())
            maxNameLength = std::max(maxNameLength, command.length());

        maxNameLength = std::min(maxNameLength, maxAllowedNameLength);

        context.log() << std::setw(maxNameLength) << std::left << "Command name" << '\t' << "Description" << std::endl;
        context.log() << std::setw(maxNameLength) << std::left << "------------" << '\t' << "-----------" << std::endl;
        for (auto const& command : m_bindings.commands())
        {
            context.log() << std::setw(maxNameLength) << std::left << command;
            if (CommandMetadata const* metadata = getMetadata(command); metadata && !metadata->description.empty())
            {
                context.log() << '\t' << metadata->description;
            }

            context.log() << std::endl;
        }
    };

    commands["clear"].description("Clears the console") = coil::bind(&DebugConsole::clear, this);

    auto globalHelp = [](coil::Context context)
    {
        context.log() << std::setw(15) << std::left << "Command name" << '\t' << "Description" << std::endl;
        context.log() << std::setw(15) << std::left << "list" << '\t' << "Lists available commands" << std::endl;
        context.log() << std::setw(15) << std::left << "clear" << '\t' << "Clears the console" << std::endl;
        context.log() << std::setw(15) << std::left << "help <command>" << '\t' << "Prints command help" << std::endl;
        context.log() << std::setw(15) << std::left << "help" << '\t' << "Prints this message" << std::endl;
    };

    auto commandHelp = [this](coil::Context context, std::string_view command)
    {
        getCommandHelp(context.log(), command);
    };

    commands["help"].description("Prints global/command help").arguments({ {}, {"command_name"} }) = coil::overloaded(std::move(globalHelp), std::move(commandHelp));
}

void DebugConsole::execute(std::string_view command)
{
    for (std::string_view subcommand : CommandsList{ command })
    {
        if (subcommand.empty())
            continue;

        addLine("> " + std::string{ subcommand }, Line::Type::Input);
        m_inputHistory.push_back(std::string{ subcommand });

        coil::ExecutionResult result = m_bindings.execute(subcommand);
        for (std::string const& error : result.errors)
            addLine("  " + error, Line::Type::Error);
        for (std::string line; std::getline(result.output, line); )
            addLine("  " + line, Line::Type::Output);
        if (result.returnValue)
            addLine("  -> '" + *result.returnValue + "'", Line::Type::ReturnValue);

        if (!result.errors.empty())
        {
            std::stringstream ss;
            getCommandHelp(ss, result.input.name);
            for (std::string line; std::getline(ss, line); )
                addLine("  " + line, Line::Type::Output);
        }
    }
}

std::vector<DebugConsole::Suggestion> DebugConsole::getSuggestions(std::string_view input) const
{
    if (input.empty())
        return {};

    std::vector<Suggestion> suggestions;

    rapidfuzz::fuzz::CachedRatio<char> scorer(input);
    for (std::string_view command : m_bindings.commands())
    {
        float score = 0.0f;

        if (command.starts_with(input))
            score = 200.0f; // force full-matches to be on top
        else if (command.find(input) != std::string_view::npos)
            score = 150.0f;
        else
            score = static_cast<float>(scorer.similarity(command));

        suggestions.push_back({ command, score });
    }

    std::sort(suggestions.begin(), suggestions.end(), [](Suggestion const& lhs, Suggestion const& rhs)
    {
        if (lhs.score == rhs.score)
            return lhs.command < rhs.command;
        return lhs.score > rhs.score;
    });

    return suggestions;
}

std::optional<std::string_view> DebugConsole::autoComplete(std::string_view input) const
{
    static std::vector<std::string_view> candidates;
    candidates.clear();

    for (std::string_view command : m_bindings.commands())
    {
        if (command.starts_with(input))
            candidates.push_back(command);
    }

    if (candidates.empty())
        return {};

    std::string_view baseCandidate = candidates[0];

    std::string_view longestPrefix = baseCandidate.substr(0, input.size());

    for (std::size_t s = input.size() + 1; s <= baseCandidate.size(); s++)
    {
        std::string_view prefix = baseCandidate.substr(0, s);

        bool isPrefixValid = true;
        for (std::string_view candidate : candidates)
            if (!candidate.starts_with(prefix))
                return longestPrefix;

        longestPrefix = prefix;
    }

    return longestPrefix;
}

void DebugConsole::clear()
{
    m_lines.clear();
}

void DebugConsole::remove(std::string_view name)
{
    m_bindings.remove(name);
    m_metadata.erase(name);
}

CommandProxy<DebugConsole> DebugConsole::operator[](std::string_view name)
{
    return { *this, name };
}

CommandMetadata const* DebugConsole::getMetadata(std::string_view name) const
{
    auto it = m_metadata.find(name);
    if (it != m_metadata.end())
        return &it->second;

    return nullptr;
}

void DebugConsole::getCommandHelp(std::ostream& os, std::string_view name) const
{
    auto const* functors = m_bindings.get(name);
    if (!functors)
    {
        os << "Command '" << name << "' doesn't exist" << std::endl;
        return;
    }

    CommandMetadata const* metadata = getMetadata(name);

    if (metadata && !metadata->description.empty())
        os << "Description: " << metadata->description << std::endl;

    bool isVariable = false;
    std::string_view variableType;

    // TODO fix this fragile logic
    if (functors->size() == 2)
    {
        auto const& types0 = (*functors)[0].parameterTypes();
        auto const& types1 = (*functors)[1].parameterTypes();

        if (types0.size() == 0 && types1.size() == 1)
        {
            isVariable = true;
            variableType = types1[0];
        }
        if (types0.size() == 1 && types1.size() == 0)
        {
            isVariable = true;
            variableType = types0[0];
        }
    }

    if (isVariable)
    {
        os << "Type: Variable (" << variableType << ")" << std::endl;
    }
    else
    {
        os << "Type: Command";

        if (functors->size() != 1 || !functors->front().parameterTypes().empty())
        {
            os << " (";

            std::string_view functorSeparator = "";
            for (std::size_t i = 0; i < functors->size(); i++)
            {
                auto const& functor = (*functors)[i];
                os << functorSeparator << '[';
                std::string_view typeSeparator = "";
                auto const& types = functor.parameterTypes();
                for (std::size_t j = 0; j < types.size(); j++)
                {
                    std::string_view type = types[j];
                    std::string_view name;
                    if (metadata)
                    {
                        auto const& names = metadata->positionalParameterNames;
                        if (names.size() > i && names[i].size() > j)
                            name = names[i][j];
                    }
                    os << typeSeparator << type;
                    if (!name.empty())
                        os << " " << name;
                    typeSeparator = ", ";
                }
                os << ']';
                functorSeparator = ", ";
            }

            os << ")";
        }

        os << std::endl;
    }
}

void DebugConsole::addLine(std::string text, Line::Type type)
{
    m_lines.push_back({ std::move(text), type });
}
