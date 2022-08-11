#pragma once

#include "CommandMetadata.h"
#include "CommandProxy.h"

#include "coil/Coil.h"
#include "magic_enum.hpp"

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

		static Expected<glm::vec3, std::string> fromString(Value const& input)
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

		static std::string toString(glm::vec3 const& value)
		{
            std::stringstream ss;

            ss << "(";
            std::string_view separator = "";

            for (std::size_t i = 0; i < N; i++)
            {
                ss << separator << value[i];
                separator = ", ";
            }

            ss << ")";

            return ss.str();
		}
	};

    template<typename T, typename>
    struct TypeName
    {
        static std::string_view name()
        {
            return typeid(T).name();
        }
    };

    template<typename E>
    struct TypeName<E, std::enable_if_t<std::is_enum_v<E>>>
    {
        static std::string_view name()
        {
            std::string_view typeName = magic_enum::enum_type_name<E>();
            auto it = typeName.rfind("::");
            if (it != std::string_view::npos)
                typeName = typeName.substr(it + 2);
            return typeName;
        }
    };

	COIL_CREATE_TYPE_NAME_DECLARATION(void);
	COIL_CREATE_TYPE_NAME_DECLARATION(std::int8_t);
	COIL_CREATE_TYPE_NAME_DECLARATION(std::uint8_t);
	COIL_CREATE_TYPE_NAME_DECLARATION(std::int16_t);
	COIL_CREATE_TYPE_NAME_DECLARATION(std::uint16_t);
	COIL_CREATE_TYPE_NAME_DECLARATION(std::int32_t);
	COIL_CREATE_TYPE_NAME_DECLARATION(std::uint32_t);
	COIL_CREATE_TYPE_NAME_DECLARATION(std::int64_t);
	COIL_CREATE_TYPE_NAME_DECLARATION(std::uint64_t);
	COIL_CREATE_TYPE_NAME_DECLARATION(float);
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
        coil::Bindings::Command const& command = m_bindings.add(name, std::forward<Functor>(functor));
        auto it = m_metadata.insert_or_assign(name, std::move(metadata)).first;

        fillCommandMetadata(it->second, command.functors);
    }

    void remove(std::string_view name);

    CommandProxy<DebugConsoleService> operator[](std::string_view name);

    CommandMetadata const* getMetadata(std::string_view name) const;

private:
    void fillCommandMetadata(CommandMetadata& metadata, std::vector<coil::AnyFunctor> const& functors);
    void addLine(std::string text, Line::Type type);

private:
    coil::Bindings m_bindings;
    std::unordered_map<coil::BasicStringWrapper<std::string>, CommandMetadata> m_metadata;

    std::vector<Line> m_lines;
    std::vector<std::string> m_inputHistory;
};
