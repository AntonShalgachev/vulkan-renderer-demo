#pragma once

#include "CommandMetadata.h"
#include "CommandProxy.h"
#include "ServiceContainer.h"

#include "nstl/string_view.h"
#include "nstl/string.h"
#include "nstl/optional.h"
#include "nstl/vector.h"

#include "coil/Coil.h"

namespace nstl
{
    class string_builder;
}

// TODO move to other file
namespace utils
{
    // TODO move?
    inline nstl::string coilToNstlString(coil::StringView str)
    {
        return { str.data(), str.length() };
    }

    inline nstl::string_view coilToNstlStringView(coil::StringView str)
    {
        return { str.data(), str.length() };
    }

    inline coil::String nstlToCoilString(nstl::string_view str)
    {
        return { str.data(), str.length() };
    }

    inline coil::StringView nstlToCoilStringView(nstl::string_view str)
    {
        return { str.data(), str.length() };
    }
}

// TODO move to some other file
namespace coil
{
    template<typename T, typename>
    struct TypeName
    {
        static coil::StringView name()
        {
            return typeid(T).name();
        }
    };
}

// TODO move out of coil namespace?
// conversion helpers
namespace coil
{
    nstl::string toNstlString(coil::StringView str);
    nstl::string_view toNstlStringView(coil::StringView str);
    coil::String fromNstlString(nstl::string_view str);
    coil::StringView fromNstlStringView(nstl::string_view str);
}

// nstl::string
namespace coil
{
    template<>
    struct TypeSerializer<nstl::string>
    {
        static Expected<nstl::string, String> fromString(Value const& input);
        static String toString(nstl::string const& value);
    };
}
COIL_CREATE_TYPE_NAME(nstl::string, "string");

// nstl::string_view
namespace coil
{
    template<>
    struct TypeSerializer<nstl::string_view>
    {
        static Expected<nstl::string_view, String> fromString(Value const& input);
        static String toString(nstl::string_view const& value);
    };
}
COIL_CREATE_TYPE_NAME(nstl::string_view, "string");

class DebugConsoleService : public ServiceContainer
{
public:
    struct Line
    {
        enum class Type
        {
            Input,
            ReturnValue,
            Output,
            Error,
            CommandHelp,
        };

        nstl::string text;
        Type type;
    };

    struct Suggestion
    {
        nstl::string_view command;
        size_t distance;
    };

    DebugConsoleService(Services& services);

    void execute(nstl::string_view command);
    nstl::vector<Suggestion> getSuggestions(nstl::string_view input) const;
    nstl::optional<nstl::string_view> autoComplete(nstl::string_view input) const;
    void clear();

	void getCommandHelp(nstl::string_builder& builder, nstl::string_view name) const;

    nstl::vector<Line> const& lines() { return m_lines; }
    nstl::vector<nstl::string> const& history() { return m_inputHistory; }

    template<typename Functor>
    void add(nstl::string_view name, CommandMetadata metadata, Functor functor)
    {
        // TODO move it somewhere, preferably coil
        static_assert(coil::detail::FuncTraits<Functor>::isFunc, "Func should be a functor object");
        using FunctionWrapper = typename coil::detail::FuncTraits<Functor>::FunctionWrapperType;
        return add(name, std::move(metadata), coil::AnyFunctor{ FunctionWrapper{coil::move(functor)} });
    }
    void add(nstl::string_view name, CommandMetadata metadata, coil::AnyFunctor anyFunctor);
    void add(nstl::string_view name, CommandMetadata metadata, coil::Vector<coil::AnyFunctor> anyFunctors);

    void remove(nstl::string_view name);

    CommandProxy<DebugConsoleService> operator[](nstl::string_view name);

    CommandMetadata const* getMetadata(nstl::string_view name) const;

private:
    void fillCommandMetadata(CommandMetadata& metadata, coil::Vector<coil::AnyFunctor> const& functors);
    void addLine(nstl::string text, Line::Type type);

private:
    coil::Bindings m_bindings;
    coil::UnorderedMap<coil::String, CommandMetadata> m_metadata;
    coil::Vector<coil::String> m_commands;

    nstl::vector<Line> m_lines;
    nstl::vector<nstl::string> m_inputHistory;
};

// TODO find the right way to do it
extern template class CommandProxy<DebugConsoleService>;
