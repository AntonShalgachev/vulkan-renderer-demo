#include "DebugConsole.h"

#include "rapidfuzz/fuzz.hpp"

#include <iomanip>

DebugConsole& DebugConsole::instance()
{
    static DebugConsole console;
    return console;
}

DebugConsole::DebugConsole()
{
    m_bindings["list"] = [this](coil::Context context)
    {
        context.out() << "Available commands:" << std::endl;
        for (auto const& command : m_bindings.commands())
            context.out() << command << std::endl;
    };

    m_bindings["clear"] = coil::bind(&DebugConsole::clear, this);

    m_bindings["help"] = [](coil::Context context)
    {
        context.out() << std::setw(15) << std::left << "Command name" << '\t' << "Description" << std::endl;
        context.out() << std::setw(15) << std::left << "list" << '\t' << "Lists available commands" << std::endl;
        context.out() << std::setw(15) << std::left << "clear" << '\t' << "Clears the console" << std::endl;
        context.out() << std::setw(15) << std::left << "help" << '\t' << "Prints this message" << std::endl;
    };
}

void DebugConsole::execute(std::string_view command)
{
    addLine("> " + std::string{ command }, Line::Type::Input);
    m_inputHistory.push_back(std::string{ command });

    coil::ExecutionResult result = m_bindings.execute(command);
    for (std::string const& error : result.errors)
        addLine("  " + error, Line::Type::Error);
    for (std::string line; std::getline(result.output, line); )
        addLine("  " + line, Line::Type::Output);
    if (result.returnValue)
        addLine("  -> '" + *result.returnValue + "'", Line::Type::ReturnValue);
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

void DebugConsole::addLine(std::string text, Line::Type type)
{
    m_lines.push_back({ std::move(text), type });
}
