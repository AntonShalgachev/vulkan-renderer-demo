#pragma once

#include "common/tiny_ctti.h"

#include "coil/Coil.h" // TODO don't include whole Coil

#include <ctype.h>

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
    struct TypeSerializer<E, nstl::enable_if_t<nstl::is_enum_v<E>>>
    {
        static Expected<E, coil::String> fromString(Value const& input)
        {
            if (input.subvalues.size() != 1)
                return errors::createMismatchedSubvaluesError<E>(input, 1);

            nstl::string_view value = coil::toNstlStringView(input.subvalues[0]);

            nstl::optional<E> optionalValue = tiny_ctti::enum_cast<E>(value, ::utils::caseInsensitivePredicate);
            if (!optionalValue)
            {
                nstl::string names = ::utils::flatten(tiny_ctti::enum_names<E>(), "'");
                return errors::createGenericError<E>(input, coil::sprintf("Possible values are [%s]", names.c_str()));
            }

            return *optionalValue;
        }

        static coil::String toString(E const& value)
        {
            return coil::fromNstlString(tiny_ctti::enum_name(value));
        }
    };

    template<typename E>
    struct TypeName<E, nstl::enable_if_t<nstl::is_enum_v<E>>>
    {
        static coil::StringView name()
        {
            return coil::fromNstlStringView(tiny_ctti::type_name<E>());
        }
    };
}
