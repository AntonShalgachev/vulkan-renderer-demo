#include "GlmSerializer.h"

#include "coil/detail/StringConv.h" // TODO remove detail include

#include "nstl/utility.h"

#include "glm.h"

coil::Expected<glm::vec3, coil::String> coil::TypeSerializer<glm::vec3>::fromString(Value const& input)
{
    if (input.subvalues.size() != N)
        return errors::createMismatchedSubvaluesError<glm::vec3>(input, N);

    ElementType values[N];
    for (size_t i = 0; i < N; i++)
    {
        auto maybeValue = TypeSerializer<ElementType>::fromString(input.subvalues[i]);
        if (!maybeValue)
            return errors::createGenericError<glm::vec3>(input, maybeValue.error());

        values[i] = *nstl::move(maybeValue);
    }

    return glm::make_vec3(values);
}

coil::String coil::TypeSerializer<glm::vec3>::toString(glm::vec3 const& value)
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

COIL_CREATE_TYPE_NAME_DEFINITION(glm::vec3, "vec3");
