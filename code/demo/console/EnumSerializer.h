#pragma once

#include "coil/Coil.h"
#include "magic_enum.hpp"

#include <string>
#include <string_view>

namespace utils
{
    template<typename StringVectorLike>
    std::string flatten(StringVectorLike const& strings, std::string_view decorator = "", std::string_view separator = ", ")
    {
        std::string_view currentSeparator = "";

        std::string result;

        for (auto const& value : strings)
        {
            result += currentSeparator;
            result += decorator;
            result += value;
            result += decorator;
            currentSeparator = separator;
        }

        return result;
    }
}

namespace coil
{
    template<typename E>
    struct TypeSerializer<E, std::enable_if_t<std::is_enum_v<E>>>
    {
        static Expected<E, coil::String> fromString(Value const& input)
        {
            if (input.subvalues.size() != 1)
                return errors::createMismatchedSubvaluesError<E>(input, 1);

            std::string_view value{ input.subvalues[0].data(), input.subvalues[0].length() };

            auto pred = [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); };
            std::optional<E> optionalValue = magic_enum::enum_cast<E>(value, std::move(pred));

            if (optionalValue.has_value())
                return optionalValue.value();

            std::string names = ::utils::flatten(magic_enum::enum_names<E>(), "'");

            return errors::createGenericError<E>(input, coil::sprintf("Possible values are [%s]", names.c_str()));
        }

        static coil::String toString(E const& value)
        {
            std::string_view name = magic_enum::enum_name(value);
            return coil::StringView{ name.data(), name.size() };
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
}
