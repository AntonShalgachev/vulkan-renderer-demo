#include "Uuid.h"

#include "platform/uuid.h"

#include "nstl/string.h"
#include "nstl/sprintf.h"

editor::assets::Uuid editor::assets::Uuid::generate()
{
    Uuid id;

    platform::uuid_generate(id.bytes);

    return id;
}

nstl::string editor::assets::Uuid::toString() const
{
    return platform::uuid_to_string(bytes);
}

size_t nstl::hash<editor::assets::Uuid>::operator()(editor::assets::Uuid const& value) const
{
    return nstl::hash_values(value.bytes);
}
