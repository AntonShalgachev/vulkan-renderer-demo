#pragma once

#include "CommandMetadata.h"
#include "CommandProxy.h"
#include "ServiceContainer.h"

#include "nstl/string_view.h"
#include "nstl/string.h"
#include "nstl/optional.h"

#include "coil/Coil.h"
#include "coil/StdLibCompat.h"

#include "magic_enum.hpp"

#include "glm.h"

// TODO move to other file
namespace utils
{
    template<typename StringVectorLike>
    std::string flatten(StringVectorLike const& strings, std::string_view decorator = "", std::string_view separator = ", ")
    {
        std::string_view currentSeparator = "";
        std::stringstream ss;

        for (auto const& value : strings)
        {
            ss << currentSeparator << decorator << value << decorator;
            currentSeparator = separator;
        }

        return ss.str();
    }

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
    template<typename E>
    struct TypeSerializer<E, std::enable_if_t<std::is_enum_v<E>>>
    {
        static Expected<E, std::string> fromString(Value const& input)
        {
            if (input.subvalues.size() != 1)
                return errors::createMismatchedSubvaluesError<E>(input, 1);

            auto value = input.subvalues[0];

            auto pred = [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); };
            std::optional<E> optionalValue = magic_enum::enum_cast<E>(value, std::move(pred));

            if (optionalValue.has_value())
                return optionalValue.value();

            std::string names = ::utils::flatten(magic_enum::enum_names<E>(), "'");

            return errors::createGenericError<E>(input, coil::sprintf("Possible values are [%s]", names.c_str()));
        }

        static std::string toString(E const& value)
        {
            return magic_enum::enum_name(value);
        }
	};

    // TODO make it templated
    template<>
    struct TypeSerializer<glm::vec3>
    {
        static std::size_t const N = 3;
        using ElementType = float;

        static Expected<glm::vec3, coil::String> fromString(Value const& input)
        {
            if (input.subvalues.size() != N)
                return errors::createMismatchedSubvaluesError<glm::vec3>(input, N);

            std::array<ElementType, N> values;
            for (std::size_t i = 0; i < N; i++)
            {
                auto maybeValue = TypeSerializer<ElementType>::fromString(input.subvalues[i]);
                if (!maybeValue)
                    return errors::createGenericError<glm::vec3>(input, maybeValue.error());

                values[i] = *std::move(maybeValue);
            }

            return glm::make_vec3(values.data());
        }

        static coil::String toString(glm::vec3 const& value)
        {
            coil::String result = "(";
            coil::StringView separator = "";

            for (std::size_t i = 0; i < N; i++)
            {
                result += separator;
                result += coil::toString(value[i]);
                separator = ", ";
            }

            result += ")";
            return result;
        }
    };

    template<typename T, typename>
    struct TypeName
    {
        static coil::StringView name()
        {
            return typeid(T).name();
        }
    };

    template<typename E>
    struct TypeName<E, std::enable_if_t<std::is_enum_v<E>>>
    {
        static coil::StringView name()
        {
            std::string_view typeName = magic_enum::enum_type_name<E>();
            auto it = typeName.rfind("::");
            if (it != std::string_view::npos)
                typeName = typeName.substr(it + 2);
            return coil::StringView{ typeName.data(), typeName.size() };
        }
    };

	COIL_CREATE_TYPE_NAME_DECLARATION(glm::vec3);
}

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
        float score;
    };

    DebugConsoleService(Services& services);

    void execute(nstl::string_view command);
    std::vector<Suggestion> getSuggestions(nstl::string_view input) const;
    nstl::optional<nstl::string_view> autoComplete(nstl::string_view input) const;
    void clear();

	void getCommandHelp(std::ostream& os, nstl::string_view name) const;

    std::vector<Line> const& lines() { return m_lines; }
    std::vector<nstl::string> const& history() { return m_inputHistory; }

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

    std::vector<Line> m_lines;
    std::vector<nstl::string> m_inputHistory;
};

// TODO find the right way to do it
extern template class CommandProxy<DebugConsoleService>;
