#include "EditorSerializer.h"

#include "services/DebugConsoleService.h" // TODO remove? Used for toNstlStringView

coil::Expected<editor::assets::Uuid, coil::String> coil::TypeSerializer<editor::assets::Uuid>::fromString(Value const& input)
{
    if (input.subvalues.size() != 1)
        return errors::createMismatchedSubvaluesError<editor::assets::Uuid>(input, 1);

    nstl::string_view str = coil::toNstlStringView(input.subvalues[0]);

    editor::assets::Uuid result;
    if (editor::assets::tryParseUuid(str, result))
        return result;

    return errors::createGenericError<editor::assets::Uuid>(input, "Input is not a UUID string (xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx)");
}

coil::String coil::TypeSerializer<editor::assets::Uuid>::toString(editor::assets::Uuid const& value)
{
    return coil::fromNstlString(value.toString());
}

COIL_CREATE_TYPE_NAME_DEFINITION(editor::assets::Uuid, "uuid");
