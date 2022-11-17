#include "DebugConsoleService.h"

#include "rapidfuzz/fuzz.hpp"

#include <iomanip>

namespace
{
    class CommandsListIterator
    {
    public:
        CommandsListIterator() = default;
        CommandsListIterator(nstl::string_view commands)
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

        nstl::string_view operator*()
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
        std::optional<nstl::string_view> m_self;
        nstl::string_view m_rest;
    };

    class CommandsList
    {
    public:
        CommandsList(nstl::string_view commands) : m_commands(commands) {}

        auto begin()
        {
            return CommandsListIterator{ m_commands };
        }

        auto end()
        {
            return CommandsListIterator{};
        }

    private:
        nstl::string_view m_commands;
    };
}

namespace coil
{
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
            maxNameLength = std::max(maxNameLength, command.length());

        maxNameLength = std::min(maxNameLength, maxAllowedNameLength);

        context.loglinef("%-*s\t%s", maxNameLength, "Command name", "Description");
        context.loglinef("%-*s\t%s", maxNameLength, "------------", "-----------");

        for (coil::String const& command : m_commands)
        {
            context.logf("%-*s", maxNameLength, command.cStr());
            if (CommandMetadata const* metadata = getMetadata(utils::coilToNstlStringView(command)); metadata && !metadata->description.empty())
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

    auto commandHelp = [this](coil::Context context, coil::StringView command)
    {
        std::stringstream ss;
        getCommandHelp(ss, utils::coilToNstlStringView(command));
        context.logf("%s", ss.str().c_str());// TODO fix this hack
    };

    commands["help"].description("Print global/command help").arguments({ {}, {"command_name"} }) = coil::overloaded(std::move(globalHelp), std::move(commandHelp));
}

void DebugConsoleService::execute(nstl::string_view command)
{
    for (nstl::string_view subcommand : CommandsList{ command })
    {
        if (subcommand.empty())
            continue;

        addLine("> " + nstl::string{ subcommand }, Line::Type::Input);

        if (m_inputHistory.empty() || m_inputHistory.back() != subcommand)
            m_inputHistory.push_back(nstl::string{ subcommand });

        coil::ExecutionResult result = m_bindings.execute(utils::nstlToCoilStringView(subcommand));
        for (coil::String const& error : result.errors)
            addLine("  " + utils::coilToNstlString(error), Line::Type::Error);

        // TODO not the best way to split the string
        std::stringstream ss{ coil::toStdString(result.output) };
        for (std::string line; std::getline(ss, line); )
            addLine("  " + nstl::string{ line.c_str() }, Line::Type::Output); // TODO fix this hack

        if (result.returnValue)
            addLine("  -> '" + utils::coilToNstlString(*result.returnValue) + "'", Line::Type::ReturnValue);

        if (!result.errors.empty())
        {
            addLine("  See command help:", Line::Type::CommandHelp);
            std::stringstream ss;
            getCommandHelp(ss, utils::coilToNstlStringView(result.input.name));
            for (std::string line; std::getline(ss, line); )
                addLine("    " + nstl::string{ line.c_str() }, Line::Type::CommandHelp);; // TODO fix this hack
        }
    }
}

std::vector<DebugConsoleService::Suggestion> DebugConsoleService::getSuggestions(nstl::string_view input) const
{
    if (input.empty())
        return {};

    std::vector<Suggestion> suggestions;

    rapidfuzz::fuzz::CachedRatio<char> scorer(input);
    for (coil::String const& coilCommand : m_commands)
    {
        nstl::string_view command = utils::coilToNstlStringView(coilCommand);

        float score = 0.0f;

        if (command.starts_with(input))
            score = 200.0f; // force full-matches to be on top
        else if (command.find(input) != nstl::string_view::npos)
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

std::optional<nstl::string_view> DebugConsoleService::autoComplete(nstl::string_view input) const
{
    static std::vector<nstl::string_view> candidates;
    candidates.clear();

    for (coil::String const& coilCommand : m_commands)
    {
        nstl::string_view command = utils::coilToNstlStringView(coilCommand);

        if (command.starts_with(input))
            candidates.push_back(command);
    }

    if (candidates.empty())
        return {};

    nstl::string_view baseCandidate = candidates[0];

    nstl::string_view longestPrefix = baseCandidate.substr(0, input.size());

    for (std::size_t s = input.size() + 1; s <= baseCandidate.size(); s++)
    {
        nstl::string_view prefix = baseCandidate.substr(0, s);

        bool isPrefixValid = true;
        for (nstl::string_view candidate : candidates)
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

void DebugConsoleService::getCommandHelp(std::ostream& os, nstl::string_view name) const
{
    // TODO fix this hack
    auto toStdStringView = [](nstl::string_view sv)
    {
        return std::string_view{ sv.data(), sv.size() };
    };

	auto const* functors = m_bindings.get(utils::nstlToCoilStringView(name));
	if (!functors)
	{
		os << "Command '" << toStdStringView(name) << "' doesn't exist" << std::endl;
		return;
	}

	CommandMetadata const* metadata = getMetadata(name);

	if (!metadata)
		return;

	if (!metadata->type.empty())
		os << "Type: " << toStdStringView(metadata->type) << std::endl;

	if (!metadata->description.empty())
		os << "Description: " << toStdStringView(metadata->description) << std::endl;

	os << "Usage:" << std::endl;
	for (FunctorMetadata const& functor : metadata->functors)
		os << "  " << toStdStringView(functor.buildRepresentation(name)) << std::endl;
}

void DebugConsoleService::add(nstl::string_view name, CommandMetadata metadata, coil::AnyFunctor anyFunctor)
{
    coil::Vector<coil::AnyFunctor> functors;
    functors.pushBack(std::move(anyFunctor));
    return add(name, std::move(metadata), std::move(functors));
}

void DebugConsoleService::add(nstl::string_view name, CommandMetadata metadata, coil::Vector<coil::AnyFunctor> anyFunctors)
{
    coil::StringView coilName = utils::nstlToCoilStringView(name);
    coil::Bindings::Command const& command = m_bindings.add(coilName, std::move(anyFunctors));
    auto it = m_metadata.insertOrAssign(coilName, std::move(metadata));

    fillCommandMetadata(it->value(), command.functors);

    m_commands.pushBack(coilName);
}

void DebugConsoleService::remove(nstl::string_view name)
{
    coil::StringView coilName = utils::nstlToCoilStringView(name);

    m_bindings.remove(coilName);
    m_metadata.erase(coilName);
}

CommandProxy<DebugConsoleService> DebugConsoleService::operator[](nstl::string_view name)
{
    return { *this, name };
}

CommandMetadata const* DebugConsoleService::getMetadata(nstl::string_view name) const
{
    coil::StringView coilName = utils::nstlToCoilStringView(name);

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
			argumentMetadata.type = utils::coilToNstlStringView(functorArgTypes[j]);
		}

		functorMetadata.returnType = utils::coilToNstlStringView(functor.returnType());
	}

	metadata.type = "Function";
    if (metadata.functors.size() == 2)
    {
        FunctorMetadata const* functor0 = &metadata.functors[0];
        FunctorMetadata const* functor1 = &metadata.functors[1];

        if (functor0->arguments.size() > functor1->arguments.size())
            std::swap(functor0, functor1);

        nstl::string_view ret0 = functor0->returnType;
        nstl::string_view ret1 = functor1->returnType;
        auto const& args0 = functor0->arguments;
        auto const& args1 = functor1->arguments;

        // TODO seems fragile
        if (args0.size() == 0 && args1.size() == 1 && ret0 == args1[0].type && ret1 == args1[0].type)
        {
            metadata.type = "Variable (" + functor1->returnType + ")";
		}
	}
}

void DebugConsoleService::addLine(nstl::string text, Line::Type type)
{
    m_lines.push_back({ std::move(text), type });
}

template class CommandProxy<DebugConsoleService>;
