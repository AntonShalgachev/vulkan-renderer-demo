#include "DebugConsoleService.h"

#include "common/Utils.h"

#include "nstl/algorithm.h"
#include "nstl/sort.h"
#include "nstl/string_builder.h"

#include "memory/tracking.h"

namespace
{
    auto consoleScopeId = memory::tracking::create_scope_id("System/Services/Console");

    class CommandsListIterator
    {
    public:
        CommandsListIterator() = default;
        CommandsListIterator(nstl::string_view commands)
        {
            if (commands.empty())
                return;

            size_t i = 0;
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
        nstl::optional<nstl::string_view> m_self;
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

    size_t levenshteinDistance(nstl::string_view lhs, nstl::string_view rhs)
    {
        if (lhs.empty())
            return rhs.length();

        if (rhs.empty())
            return lhs.length();

        if (lhs == rhs)
            return 0;

        nstl::vector<size_t> cache;
        cache.resize(lhs.length());

        for (size_t i = 0; i < lhs.length(); i++)
            cache[i] = i + 1;

        size_t result = 0;
        for (size_t bIndex = 0; bIndex < rhs.length(); bIndex++)
        {
            char c = rhs[bIndex];
            result = bIndex;
            size_t distance = bIndex;

            for (size_t index = 0; index < lhs.length(); index++)
            {
                size_t bDistance = (c == lhs[index]) ? distance : distance + 1;
                distance = cache[index];

                cache[index] = result = distance > result
                    ? bDistance > result
                    ? result + 1
                    : bDistance
                    : bDistance > distance
                    ? distance + 1
                    : bDistance;
            }
        }

        return result;
    }
}

// conversion helpers
nstl::string coil::toNstlString(StringView str)
{
    return { str.data(), str.length() };
}

nstl::string_view coil::toNstlStringView(StringView str)
{
    return { str.data(), str.length() };
}

coil::String coil::fromNstlString(nstl::string_view str)
{
    return { str.data(), str.length() };
}

coil::StringView coil::fromNstlStringView(nstl::string_view str)
{
    return { str.data(), str.length() };
}

// nstl::string
coil::Expected<nstl::string, coil::String> coil::TypeSerializer<nstl::string>::fromString(coil::Value const& input)
{
    if (input.subvalues.size() != 1)
        return errors::createMismatchedSubvaluesError<nstl::string>(input, 1);

    return toNstlString(input.subvalues[0]);
}

coil::String coil::TypeSerializer<nstl::string>::toString(nstl::string const& value)
{
    return fromNstlString(value);
}

// nstl::string_view
coil::Expected<nstl::string_view, coil::String> coil::TypeSerializer<nstl::string_view>::fromString(coil::Value const& input)
{
    if (input.subvalues.size() != 1)
        return errors::createMismatchedSubvaluesError<nstl::string_view>(input, 1);

    return toNstlStringView(input.subvalues[0]);
}

coil::String coil::TypeSerializer<nstl::string_view>::toString(nstl::string_view const& value)
{
    if (value.empty())
        return coil::String{};

    return fromNstlString(value);
}

DebugConsoleService::DebugConsoleService(Services& services) : ServiceContainer(services)
{
    MEMORY_TRACKING_SCOPE(consoleScopeId);

    auto& commands = *this;

    commands["list"].description("List available commands") = [this](coil::Context context)
    {
        int maxAllowedNameLength = 50;

        int maxNameLength = 0;
        for (coil::String const& command : m_commands)
            maxNameLength = nstl::max(maxNameLength, command.slength());

        maxNameLength = nstl::min(maxNameLength, maxAllowedNameLength);

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
        nstl::string_builder builder;
        getCommandHelp(builder, utils::coilToNstlStringView(command));
        context.logf("%s", builder.build().c_str());
    };

    commands["help"].description("Print global/command help").arguments({ {}, {"command_name"} }) = coil::overloaded(nstl::move(globalHelp), nstl::move(commandHelp));
}

bool DebugConsoleService::execute(nstl::string_view command)
{
    MEMORY_TRACKING_SCOPE(consoleScopeId);

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

        if (!result.output.empty())
        {
            for (nstl::string_view line : vkc::utils::split(utils::coilToNstlStringView(result.output)))
                addLine("  " + nstl::string{ line }, Line::Type::Output);
        }

        if (result.returnValue)
            addLine("  -> '" + utils::coilToNstlString(*result.returnValue) + "'", Line::Type::ReturnValue);

        if (!result.errors.empty())
        {
            addLine("  See command help:", Line::Type::CommandHelp);
            nstl::string_builder builder;
            getCommandHelp(builder, utils::coilToNstlStringView(result.input.name));
            nstl::string commandHelp = builder.build();
            for (nstl::string_view line : vkc::utils::split(commandHelp))
                addLine("    " + nstl::string{ line }, Line::Type::CommandHelp);

            return false;
        }
    }

    return true;
}

nstl::vector<DebugConsoleService::Suggestion> DebugConsoleService::getSuggestions(nstl::string_view input) const
{
    MEMORY_TRACKING_SCOPE(consoleScopeId);

    if (input.empty())
        return {};

    nstl::vector<Suggestion> suggestions;

    for (coil::String const& coilCommand : m_commands)
    {
        nstl::string_view command = utils::coilToNstlStringView(coilCommand);

        size_t distance = 0;

        if (command.starts_with(input))
            distance = 0;
        else if (command.find(input) != nstl::string_view::npos)
            distance = 0;
        else
            distance = levenshteinDistance(input, command);

        suggestions.push_back({ command, distance });
    }

    nstl::simple_sort(suggestions.begin(), suggestions.end(), [](Suggestion const& lhs, Suggestion const& rhs)
    {
        if (lhs.distance == rhs.distance)
            return lhs.command < rhs.command;
        return lhs.distance < rhs.distance;
    });

    return suggestions;
}

nstl::optional<nstl::string_view> DebugConsoleService::autoComplete(nstl::string_view input) const
{
    MEMORY_TRACKING_SCOPE(consoleScopeId);

    static nstl::vector<nstl::string_view> candidates;
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

    for (size_t s = input.size() + 1; s <= baseCandidate.size(); s++)
    {
        nstl::string_view prefix = baseCandidate.substr(0, s);

        for (nstl::string_view candidate : candidates)
            if (!candidate.starts_with(prefix))
                return longestPrefix;

        longestPrefix = prefix;
    }

    return longestPrefix;
}

void DebugConsoleService::clear()
{
    MEMORY_TRACKING_SCOPE(consoleScopeId);

    m_lines.clear();
}

void DebugConsoleService::getCommandHelp(nstl::string_builder& builder, nstl::string_view name) const
{
    MEMORY_TRACKING_SCOPE(consoleScopeId);

    auto const* functors = m_bindings.get(utils::nstlToCoilStringView(name));
    if (!functors)
    {
        builder.append("Command '").append(name).append("' doesn't exist\n");
        return;
    }

    CommandMetadata const* metadata = getMetadata(name);

    if (!metadata)
        return;

    if (!metadata->type.empty())
        builder.append("Type: ").append(metadata->type).append('\n');

    if (!metadata->description.empty())
        builder.append("Description: ").append(metadata->description).append('\n');

    builder.append("Usage:\n");
    for (FunctorMetadata const& functor : metadata->functors)
        builder.append("  ").append(functor.buildRepresentation(name)).append('\n');
}

void DebugConsoleService::add(nstl::string_view name, CommandMetadata metadata, coil::AnyFunctor anyFunctor)
{
    MEMORY_TRACKING_SCOPE(consoleScopeId);

    coil::Vector<coil::AnyFunctor> functors;
    functors.pushBack(nstl::move(anyFunctor));
    return add(name, nstl::move(metadata), nstl::move(functors));
}

void DebugConsoleService::add(nstl::string_view name, CommandMetadata metadata, coil::Vector<coil::AnyFunctor> anyFunctors)
{
    MEMORY_TRACKING_SCOPE(consoleScopeId);

    coil::StringView coilName = utils::nstlToCoilStringView(name);
    coil::Bindings::Command const& command = m_bindings.add(coilName, nstl::move(anyFunctors));
    auto it = m_metadata.insertOrAssign(coil::String{ coilName }, nstl::move(metadata));

    fillCommandMetadata(it->value(), command.functors);

    m_commands.pushBack(coil::String{ coilName });
}

void DebugConsoleService::remove(nstl::string_view name)
{
    MEMORY_TRACKING_SCOPE(consoleScopeId);

    coil::StringView coilName = utils::nstlToCoilStringView(name);

    m_bindings.remove(coilName);
    m_metadata.erase(coilName);
}

CommandProxy<DebugConsoleService> DebugConsoleService::operator[](nstl::string_view name)
{
    MEMORY_TRACKING_SCOPE(consoleScopeId);

    return { *this, name };
}

CommandMetadata const* DebugConsoleService::getMetadata(nstl::string_view name) const
{
    MEMORY_TRACKING_SCOPE(consoleScopeId);

    coil::StringView coilName = utils::nstlToCoilStringView(name);

    auto it = m_metadata.find(coilName);
    if (it != m_metadata.end())
        return &it->value();

    return nullptr;
}

void DebugConsoleService::fillCommandMetadata(CommandMetadata& metadata, coil::Vector<coil::AnyFunctor> const& functors)
{
    MEMORY_TRACKING_SCOPE(consoleScopeId);

	if (metadata.functors.size() < functors.size())
		metadata.functors.resize(functors.size());

	for (size_t i = 0; i < functors.size(); i++)
	{
		coil::AnyFunctor const& functor = functors[i];
		coil::Vector<coil::StringView> const& functorArgTypes = functor.parameterTypes();
		FunctorMetadata& functorMetadata = metadata.functors[i];

		if (functorMetadata.arguments.size() < functorArgTypes.size())
			functorMetadata.arguments.resize(functorArgTypes.size());

		for (size_t j = 0; j < functorArgTypes.size(); j++)
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
            nstl::exchange(functor0, functor1);

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
    MEMORY_TRACKING_SCOPE(consoleScopeId);

    m_lines.push_back({ nstl::move(text), type });
}

template class CommandProxy<DebugConsoleService>;
