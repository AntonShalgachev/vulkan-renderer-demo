#pragma once

#include "common/charming_enum.h"

#include "coil/Coil.h" // TODO don't include whole Coil

namespace utils
{
    template<typename StringVectorLike>
    nstl::string flatten(StringVectorLike const& strings, nstl::string_view decorator = "", nstl::string_view separator = ", ")
    {
        nstl::string_builder builder;

        nstl::string_view currentSeparator = "";

        for (auto const& value : strings)
        {
            builder.append(currentSeparator).append(decorator).append(value).append(decorator);
            currentSeparator = separator;
        }

        return builder.build();
    }

    bool caseInsensitivePredicate(nstl::string_view lhs, nstl::string_view rhs)
    {
        if (lhs.length() != rhs.length())
            return false;

        for (size_t i = 0; i < lhs.size(); i++)
            if (tolower(lhs[i]) != tolower(rhs[i]))
                return false;

        return true;
    };
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

            nstl::string_view value{ input.subvalues[0].data(), input.subvalues[0].length() };
            
            if (nstl::optional<E> optionalValue = charming_enum::enum_cast<E>(value, ::utils::caseInsensitivePredicate))
                return *optionalValue;

            nstl::string names = ::utils::flatten(charming_enum::enum_names<E>(), "'");

            return errors::createGenericError<E>(input, coil::sprintf("Possible values are [%s]", names.c_str()));
        }

        static coil::String toString(E const& value)
        {
            nstl::string_view name = charming_enum::enum_name(value);
            return coil::StringView{ name.data(), name.size() };
        }
    };

    template<typename E>
    struct TypeName<E, std::enable_if_t<std::is_enum_v<E>>>
    {
        static coil::StringView name()
        {
            nstl::string_view typeName = charming_enum::enum_type_name<E>();
            // TODO implement
//             auto it = typeName.rfind("::");
//             if (it != std::string_view::npos)
//                 typeName = typeName.substr(it + 2);
            return coil::StringView{ typeName.data(), typeName.size() };
        }
    };
}
