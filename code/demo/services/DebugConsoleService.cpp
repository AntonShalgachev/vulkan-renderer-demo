#include "DebugConsoleService.h"

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

namespace coil
{
	COIL_CREATE_TYPE_NAME_DEFINITION(std::string, "std::string");
	COIL_CREATE_TYPE_NAME_DEFINITION(std::string_view, "std::string_view");
	COIL_CREATE_TYPE_NAME_DEFINITION(glm::vec3, "vec3");
}

DebugConsoleService::DebugConsoleService(Services& services) : ServiceContainer(services)
{
    auto& commands = *this;

    commands["list"].description("List available commands") = [this](coil::Context context)
    {
        std::size_t maxAllowedNameLength = 50;

        std::size_t maxNameLength = 0;
        for (coil::String const& command : m_commands)
            maxNameLength = std::max(maxNameLength, command.size());

        maxNameLength = std::min(maxNameLength, maxAllowedNameLength);

        context.loglinef("%-*s\t%s", maxNameLength, "Command name", "Description");
        context.loglinef("%-*s\t%s", maxNameLength, "------------", "-----------");

        for (coil::String const& command : m_commands)
        {
            context.logf("%-*s", maxNameLength, command.cStr());
            if (CommandMetadata const* metadata = getMetadata(utils::coilToStdStringView(command)); metadata && !metadata->description.empty())
                context.logf("\t%s", metadata->description.c_str());

            context.log("\n");
        }
    };

    commands["clear"].description("Clear the console") = coil::bind(&DebugConsoleService::clear, this);

    auto globalHelp = [](coil::Context context)
    {
        context.loglinef("Command name    \tDescription");
        context.loglinef("list            \tLists available commands");
        context.loglinef("clear           \tClears the console");
        context.loglinef("help <command>  \tPrints command help");
        context.loglinef("help            \tPrints this message");
    };

    auto commandHelp = [this](coil::Context context, std::string_view command)
    {
        std::stringstream ss;
        getCommandHelp(ss, command);
        context.log(utils::stdToCoilString(ss.str()));
    };

    commands["help"].description("Print global/command help").arguments({ {}, {"command_name"} }) = coil::overloaded(std::move(globalHelp), std::move(commandHelp));
}

void DebugConsoleService::execute(std::string_view command)
{
    for (std::string_view subcommand : CommandsList{ command })
    {
        if (subcommand.empty())
            continue;

        addLine("> " + std::string{ subcommand }, Line::Type::Input);

        if (m_inputHistory.empty() || m_inputHistory.back() != subcommand)
            m_inputHistory.push_back(std::string{ subcommand });

        coil::ExecutionResult result = m_bindings.execute(utils::stdToCoilStringView(subcommand));
        for (coil::String const& error : result.errors)
            addLine("  " + utils::coilToStdString(error), Line::Type::Error);

        // TODO not the best way to split the string
        std::stringstream ss{ utils::coilToStdString(result.output) };
        for (std::string line; std::getline(ss, line); )
            addLine("  " + line, Line::Type::Output);

        if (result.returnValue)
            addLine("  -> '" + utils::coilToStdString(*result.returnValue) + "'", Line::Type::ReturnValue);

        if (!result.errors.empty())
        {
            addLine("  See command help:", Line::Type::CommandHelp);
            std::stringstream ss;
            getCommandHelp(ss, utils::coilToStdStringView(result.input.name));
            for (std::string line; std::getline(ss, line); )
                addLine("    " + line, Line::Type::CommandHelp);
        }
    }
}

std::vector<DebugConsoleService::Suggestion> DebugConsoleService::getSuggestions(std::string_view input) const
{
    if (input.empty())
        return {};

    std::vector<Suggestion> suggestions;

    rapidfuzz::fuzz::CachedRatio<char> scorer(input);
    for (coil::String const& coilCommand : m_commands)
    {
        std::string_view command = utils::coilToStdStringView(coilCommand);

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

std::optional<std::string_view> DebugConsoleService::autoComplete(std::string_view input) const
{
    static std::vector<std::string_view> candidates;
    candidates.clear();

    for (coil::String const& coilCommand : m_commands)
    {
        std::string_view command = utils::coilToStdStringView(coilCommand);

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

void DebugConsoleService::clear()
{
    m_lines.clear();
}

void DebugConsoleService::getCommandHelp(std::ostream& os, std::string_view name) const
{
	auto const* functors = m_bindings.get(utils::stdToCoilStringView(name));
	if (!functors)
	{
		os << "Command '" << name << "' doesn't exist" << std::endl;
		return;
	}

	CommandMetadata const* metadata = getMetadata(name);

	if (!metadata)
		return;

	if (!metadata->type.empty())
		os << "Type: " << metadata->type << std::endl;

	if (!metadata->description.empty())
		os << "Description: " << metadata->description << std::endl;

	os << "Usage:" << std::endl;
	for (FunctorMetadata const& functor : metadata->functors)
		os << "  " << functor.buildRepresentation(name) << std::endl;
}

void DebugConsoleService::remove(std::string_view name)
{
    coil::StringView coilName = utils::stdToCoilStringView(name);

    m_bindings.remove(coilName);
    m_metadata.erase(coilName);
}

CommandProxy<DebugConsoleService> DebugConsoleService::operator[](std::string_view name)
{
    return { *this, name };
}

CommandMetadata const* DebugConsoleService::getMetadata(std::string_view name) const
{
    coil::StringView coilName = utils::stdToCoilStringView(name);

    auto it = m_metadata.find(coilName);
    if (it != m_metadata.end())
        return &it->value();

    return nullptr;
}

void DebugConsoleService::fillCommandMetadata(CommandMetadata& metadata, coil::Vector<coil::AnyFunctor> const& functors)
{
	if (metadata.functors.size() < functors.size())
		metadata.functors.resize(functors.size());

	for (std::size_t i = 0; i < functors.size(); i++)
	{
		coil::AnyFunctor const& functor = functors[i];
		coil::Vector<coil::StringView> const& functorArgTypes = functor.parameterTypes();
		FunctorMetadata& functorMetadata = metadata.functors[i];

		if (functorMetadata.arguments.size() < functorArgTypes.size())
			functorMetadata.arguments.resize(functorArgTypes.size());

		for (std::size_t j = 0; j < functorArgTypes.size(); j++)
		{
			ArgumentMetadata& argumentMetadata = functorMetadata.arguments[j];
			argumentMetadata.type = utils::coilToStdStringView(functorArgTypes[j]);
		}

		functorMetadata.returnType = utils::coilToStdStringView(functor.returnType());
	}

	metadata.type = "Function";
    if (metadata.functors.size() == 2)
    {
        FunctorMetadata const* functor0 = &metadata.functors[0];
        FunctorMetadata const* functor1 = &metadata.functors[1];

        if (functor0->arguments.size() > functor1->arguments.size())
            std::swap(functor0, functor1);

        std::string_view ret0 = functor0->returnType;
        std::string_view ret1 = functor1->returnType;
        auto const& args0 = functor0->arguments;
        auto const& args1 = functor1->arguments;

        if (args0.size() == 0 && args1.size() == 1 && ret0 == args1[0].type && ret1 == args1[0].type)
        {
            metadata.type = "Variable (" + functor1->returnType + ")";
		}
	}
}

void DebugConsoleService::addLine(std::string text, Line::Type type)
{
    m_lines.push_back({ std::move(text), type });
}
