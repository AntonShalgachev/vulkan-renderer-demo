#include "json-glm.h"

#include "yyjsoncpp/default_serializer.h"
#include "yyjsoncpp/doc.h"
// #include "yyjsoncpp/value_ref.h"
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

            result[it.index()] = *component;
        }

        return true;
    }

    template<size_t N, typename T, glm::qualifier Q>
    bool try_parse(yyjsoncpp::value_ref obj, glm::vec<N, T, Q>& result)
    {
        return try_parse_impl<N, T>(obj, result);
    }

    template<typename T, glm::qualifier Q>
    bool try_parse(yyjsoncpp::value_ref obj, glm::qua<T, Q>& result)
    {
        return try_parse_impl<4, T>(obj, result);
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

yyjsoncpp::optional<glm::vec3> yyjsoncpp::serializer<glm::vec3>::from_json(yyjsoncpp::value_ref obj)
{
    return parse_json<glm::vec3>(obj);
}

yyjsoncpp::mutable_value_ref yyjsoncpp::serializer<glm::vec3>::to_json(yyjsoncpp::mutable_doc& doc, glm::vec3 const& value)
{
    return doc.create_array(value.x, value.y, value.z);
}

//////////////////////////////////////////////////////////////////////////

yyjsoncpp::optional<glm::vec4> yyjsoncpp::serializer<glm::vec4>::from_json(yyjsoncpp::value_ref obj)
{
    return parse_json<glm::vec4>(obj);
}

yyjsoncpp::mutable_value_ref yyjsoncpp::serializer<glm::vec4>::to_json(yyjsoncpp::mutable_doc& doc, glm::vec4 const& value)
{
    return doc.create_array(value.x, value.y, value.z, value.w);
}

//////////////////////////////////////////////////////////////////////////

yyjsoncpp::optional<glm::quat> yyjsoncpp::serializer<glm::quat>::from_json(yyjsoncpp::value_ref obj)
{
    return parse_json<glm::quat>(obj);
}

yyjsoncpp::mutable_value_ref yyjsoncpp::serializer<glm::quat>::to_json(yyjsoncpp::mutable_doc& doc, glm::quat const& value)
{
    return doc.create_array(value.x, value.y, value.z, value.w);
}
