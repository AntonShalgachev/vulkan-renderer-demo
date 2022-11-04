#pragma once

#include "CommandMetadata.h"
#include "CommandProxy.h"

#include "coil/Coil.h"
#include "magic_enum.hpp"

#include "glm.h"
#include "ServiceContainer.h"

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

    inline std::string coilToStdString(coil::StringView str)
    {
        return { str.data(), str.length() };
    }

    inline std::string_view coilToStdStringView(coil::StringView str)
    {
        return { str.data(), str.length() };
    }

    inline coil::String stdToCoilString(std::string_view str)
    {
        return { str.data(), str.length() };
    }

    inline coil::StringView stdToCoilStringView(std::string_view str)
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

            return errors::createGenericError<E>(input, formatString("Possible values are [%s]", names.c_str()));
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

    // TODO move to cpp
    template<>
    struct TypeSerializer<std::string>
    {
        static Expected<std::string, coil::String> fromString(Value const& input)
        {
            if (input.subvalues.size() != 1)
                return errors::createMismatchedSubvaluesError<String>(input, 1);

            return utils::coilToStdString(input.subvalues[0]);
        }

        static coil::String toString(std::string const& value)
        {
            return utils::stdToCoilString(value);
        }
    };

    template<>
    struct TypeSerializer<std::string_view>
    {
        static Expected<std::string_view, coil::String> fromString(Value const& input)
        {
            if (input.subvalues.size() != 1)
                return errors::createMismatchedSubvaluesError<String>(input, 1);

            return utils::coilToStdStringView(input.subvalues[0]);
        }

        static coil::String toString(std::string_view const& value)
        {
            return utils::stdToCoilString(value);
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

	COIL_CREATE_TYPE_NAME_DECLARATION(std::string);
	COIL_CREATE_TYPE_NAME_DECLARATION(std::string_view);
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

        std::string text;
        Type type;
    };

    struct Suggestion
    {
        std::string_view command;
        float score;
    };

    DebugConsoleService(Services& services);

    void execute(std::string_view command);
    std::vector<Suggestion> getSuggestions(std::string_view input) const;
    std::optional<std::string_view> autoComplete(std::string_view input) const;
    void clear();

	void getCommandHelp(std::ostream& os, std::string_view name) const;

    std::vector<Line> const& lines() { return m_lines; }
    std::vector<std::string> const& history() { return m_inputHistory; }

    template<typename Functor>
    void add(std::string_view name, CommandMetadata metadata, Functor&& functor)
    {
        coil::StringView coilName = utils::stdToCoilStringView(name);
        coil::Bindings::Command const& command = m_bindings.add(coilName, std::forward<Functor>(functor));
        auto it = m_metadata.insertOrAssign(coilName, std::move(metadata));

        fillCommandMetadata(it->value(), command.functors);

        m_commands.pushBack(coilName);
    }

    void remove(std::string_view name);

    CommandProxy<DebugConsoleService> operator[](std::string_view name);

    CommandMetadata const* getMetadata(std::string_view name) const;

private:
    void fillCommandMetadata(CommandMetadata& metadata, coil::Vector<coil::AnyFunctor> const& functors);
    void addLine(std::string text, Line::Type type);

private:
    coil::Bindings m_bindings;
    coil::UnorderedMap<coil::String, CommandMetadata> m_metadata;
    coil::Vector<coil::String> m_commands;

    std::vector<Line> m_lines;
    std::vector<std::string> m_inputHistory;
};
