#include "Uuid.h"

#include "platform/uuid.h"

#include "nstl/string.h"
#include "nstl/sprintf.h"

#include "yyjsoncpp/doc.h"
#include "yyjsoncpp/value_ref.h"
#include "yyjsoncpp/type.h"

editor::assets::Uuid::Uuid(nstl::string_view str)
{
    bool result = platform::uuid_from_string(str, bytes);
    assert(result);
}

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

yyjsoncpp::optional<editor::assets::Uuid> yyjsoncpp::serializer<editor::assets::Uuid>::from_json(yyjsoncpp::value_ref obj)
{
    if (obj.get_type() == yyjsoncpp::type::null)
        return editor::assets::Uuid{};

    if (obj.get_type() != yyjsoncpp::type::string)
        return {};

    return editor::assets::Uuid{ obj.get_string() };
}

yyjsoncpp::mutable_value_ref yyjsoncpp::serializer<editor::assets::Uuid>::to_json(yyjsoncpp::mutable_doc& doc, editor::assets::Uuid const& value)
{
    if (!value)
        return doc.create_null();

    return doc.create_string(value.toString());
}
