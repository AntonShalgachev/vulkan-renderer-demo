#pragma once

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

namespace coil
{
    template<typename E>
    struct TypeSerializer<E, std::enable_if_t<std::is_enum_v<E>>>
    {
        static Expected<E, std::string> fromString(ArgValue const& input)
        {
            if (input.subvalues.size() != 1)
                return errors::wrongSubvaluesSize<E>(input, 1);

            auto value = input.subvalues[0];

            auto pred = [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); };
            std::optional<E> optionalValue = magic_enum::enum_cast<E>(value, std::move(pred));

            if (optionalValue.has_value())
                return optionalValue.value();

            std::string names = ::utils::flatten(magic_enum::enum_names<E>(), "'");

            return errors::serializationError<E>(input, formatString("Possible values are [%s]", names.c_str()));
        }

        static std::string toString(E const& value)
        {
            return magic_enum::enum_name(value);
        }
    };

    template<typename T>
    struct TypeName<T>
    {
        static std::string_view name()
        {
            return typeid(T).name();
        }
    };
}

class DebugConsole
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
        };

        std::string text;
        Type type;
    };

    static DebugConsole& instance();

    void execute(std::string_view command);

    std::vector<Line> const& lines() { return m_lines; }
    std::vector<std::string> const& history() { return m_inputHistory; }

    coil::Bindings& bindings() { return m_bindings; }

private:
    void addLine(std::string text, Line::Type type);

private:
    coil::Bindings m_bindings;

    std::vector<Line> m_lines;
    std::vector<std::string> m_inputHistory;
};
