#include "GlmSerializer.h"

#include "coil/detail/StringConv.h" // TODO remove detail include

#include "nstl/utility.h"

#include "tglm/types.h"

namespace
{
    template<typename T, typename ElementType, size_t N>
    coil::Expected<T, coil::String> tglmFromString(coil::Value const& input)
    {
        if (input.subvalues.size() != N)
            return coil::errors::createMismatchedSubvaluesError<T>(input, N);

        T result;
        for (size_t i = 0; i < N; i++)
        {
            auto maybeValue = coil::TypeSerializer<ElementType>::fromString(input.subvalues[i]);
            if (!maybeValue)
                return coil::errors::createGenericError<T>(input, maybeValue.error());

            result[i] = *nstl::move(maybeValue);
        }

        return result;
    }

    template<typename T, typename ElementType, size_t N>
    coil::String tglmFoString(T const& value)
    {
        coil::String result = "(";
        coil::StringView separator = "";

        for (size_t i = 0; i < N; i++)
        {
            result += separator;
            result += coil::toString(value[i]);
            separator = ", ";
        }

        result += ")";
        return result;
    }
}

coil::Expected<tglm::vec2, coil::String> coil::TypeSerializer<tglm::vec2>::fromString(Value const& input)
{
    return ::tglmFromString<tglm::vec2, float, 2>(input);
}

coil::String coil::TypeSerializer<tglm::vec2>::toString(tglm::vec2 const& value)
{
    return ::tglmFoString<tglm::vec2, float, 2>(value);
}

COIL_CREATE_TYPE_NAME_DEFINITION(tglm::vec2, "vec2");

coil::Expected<tglm::vec3, coil::String> coil::TypeSerializer<tglm::vec3>::fromString(Value const& input)
{
    return ::tglmFromString<tglm::vec3, float, 3>(input);
}

coil::String coil::TypeSerializer<tglm::vec3>::toString(tglm::vec3 const& value)
{
    return ::tglmFoString<tglm::vec3, float, 3>(value);
}

COIL_CREATE_TYPE_NAME_DEFINITION(tglm::vec3, "vec3");

coil::Expected<tglm::vec4, coil::String> coil::TypeSerializer<tglm::vec4>::fromString(Value const& input)
{
    return ::tglmFromString<tglm::vec4, float, 4>(input);
}

coil::String coil::TypeSerializer<tglm::vec4>::toString(tglm::vec4 const& value)
{
    return ::tglmFoString<tglm::vec4, float, 4>(value);
}

COIL_CREATE_TYPE_NAME_DEFINITION(tglm::vec4, "vec4");
