#include "common/json-nstl.h"

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
