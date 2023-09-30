#include "common/json-glm.h"

#include "tglm/tglm.h"

#include "yyjsoncpp/default_serializer.h"
#include "yyjsoncpp/doc.h"
#include "yyjsoncpp/array_ref.h"
#include "yyjsoncpp/type.h"

namespace
{
    template<size_t N, typename T, typename Container>
    bool try_parse_impl(yyjsoncpp::value_ref obj, Container& result)
    {
        if (obj.get_type() != yyjsoncpp::type::array)
            return false;

        yyjsoncpp::array_ref array = obj.get_array();

        if (array.size() != N)
            return false;

        for (auto it = array.begin(); it != array.end(); it++)
        {
            yyjsoncpp::optional<T> component = yyjsoncpp::serializer<T>::from_json(*it);
            if (!component)
                return {};

            result.data[it.index()] = *component;
        }

        return true;
    }

    bool try_parse(yyjsoncpp::value_ref obj, tglm::vec4& result)
    {
        return try_parse_impl<4, float>(obj, result);
    }

    bool try_parse(yyjsoncpp::value_ref obj, tglm::vec3& result)
    {
        return try_parse_impl<3, float>(obj, result);
    }

    bool try_parse(yyjsoncpp::value_ref obj, tglm::quat& result)
    {
        return try_parse_impl<4, float>(obj, result);
    }

    template<typename T>
    yyjsoncpp::optional<T> parse_json(yyjsoncpp::value_ref obj)
    {
        T result;
        if (!try_parse(obj, result))
            return {};

        return result;
    }
}

yyjsoncpp::optional<tglm::vec3> yyjsoncpp::serializer<tglm::vec3>::from_json(yyjsoncpp::value_ref obj)
{
    return parse_json<tglm::vec3>(obj);
}

yyjsoncpp::mutable_value_ref yyjsoncpp::serializer<tglm::vec3>::to_json(yyjsoncpp::mutable_doc& doc, tglm::vec3 const& value)
{
    return doc.create_array(value.x, value.y, value.z);
}

//////////////////////////////////////////////////////////////////////////

yyjsoncpp::optional<tglm::vec4> yyjsoncpp::serializer<tglm::vec4>::from_json(yyjsoncpp::value_ref obj)
{
    return parse_json<tglm::vec4>(obj);
}

yyjsoncpp::mutable_value_ref yyjsoncpp::serializer<tglm::vec4>::to_json(yyjsoncpp::mutable_doc& doc, tglm::vec4 const& value)
{
    return doc.create_array(value.x, value.y, value.z, value.w);
}

//////////////////////////////////////////////////////////////////////////

yyjsoncpp::optional<tglm::quat> yyjsoncpp::serializer<tglm::quat>::from_json(yyjsoncpp::value_ref obj)
{
    return parse_json<tglm::quat>(obj);
}

yyjsoncpp::mutable_value_ref yyjsoncpp::serializer<tglm::quat>::to_json(yyjsoncpp::mutable_doc& doc, tglm::quat const& value)
{
    return doc.create_array(value.x, value.y, value.z, value.w);
}
