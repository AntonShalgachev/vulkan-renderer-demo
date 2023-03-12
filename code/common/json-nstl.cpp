#include "json-nstl.h"

#include "yyjsoncpp/value_ref.h"
#include "yyjsoncpp/type.h"
#include "yyjsoncpp/doc.h"

yyjsoncpp::optional<nstl::string> yyjsoncpp::serializer<nstl::string>::from_json(value_ref obj)
{
    if (obj.get_type() != type::string)
        return {};

    return nstl::string{ obj.get_string() };
}

yyjsoncpp::mutable_value_ref yyjsoncpp::serializer<nstl::string>::to_json(mutable_doc& doc, nstl::string const& obj)
{
    return doc.create_string(obj);
}

yyjsoncpp::optional<nstl::string_view> yyjsoncpp::serializer<nstl::string_view>::from_json(value_ref obj)
{
    if (obj.get_type() != type::string)
        return {};

    return obj.get_string();
}

yyjsoncpp::mutable_value_ref yyjsoncpp::serializer<nstl::string_view>::to_json(mutable_doc& doc, nstl::string_view const& obj)
{
    return doc.create_string(obj);
}
