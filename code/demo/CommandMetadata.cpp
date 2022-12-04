#include "CommandMetadata.h"

#include "nstl/string_builder.h"

nstl::string FunctorMetadata::buildRepresentation(nstl::string_view name) const
{
    nstl::string_builder builder;

    if (returnType != "void")
        builder.append('[').append(returnType).append("] ");

    builder.append(name);

    for (ArgumentMetadata const& argument : arguments)
    {
        nstl::string_view arg = argument.type;
        if (!argument.name.empty())
            arg = argument.name;

        builder.append(' ').append(arg);
    }

    return builder.build();
}
