#include "yyjsoncpp/value_ref.h"

#include "yyjsoncpp/type.h"
#include "yyjsoncpp/array_ref.h"
#include "yyjsoncpp/object_ref.h"

#include "yyjson.h"

namespace
{
    yyjsoncpp::type convert_type(yyjson_type type)
    {
        switch (type)
        {
        case YYJSON_TYPE_NONE: return yyjsoncpp::type::none;
        case YYJSON_TYPE_RAW: return yyjsoncpp::type::raw;
        case YYJSON_TYPE_NULL: return yyjsoncpp::type::null;
        case YYJSON_TYPE_BOOL: return yyjsoncpp::type::boolean;
        case YYJSON_TYPE_NUM: return yyjsoncpp::type::number;
        case YYJSON_TYPE_STR: return yyjsoncpp::type::string;
        case YYJSON_TYPE_ARR: return yyjsoncpp::type::array;
        case YYJSON_TYPE_OBJ: return yyjsoncpp::type::object;
        }

        assert(false);
        return yyjsoncpp::type::none;
    }
}

yyjsoncpp::value_ref::value_ref(yyjson_val* value) : m_handle(value)
{
    assert(m_handle);
}

yyjsoncpp::type yyjsoncpp::value_ref::get_type() const
{
    assert(m_handle);
    return convert_type(yyjson_get_type(m_handle));
}

yyjsoncpp::value_ref yyjsoncpp::value_ref::operator[](string_view name) const
{
    assert(get_type() == type::object);
    return get_object().get(name);
}

yyjsoncpp::array_ref yyjsoncpp::value_ref::get_array() const
{
    assert(get_type() == type::array);
    return array_ref{ m_handle };
}

yyjsoncpp::object_ref yyjsoncpp::value_ref::get_object() const
{
    assert(get_type() == type::object);
    return object_ref{ m_handle };
}

char const* yyjsoncpp::value_ref::get_cstring() const
{
    assert(get_type() == type::string);
    return yyjson_get_str(m_handle);
}

yyjsoncpp::string_view yyjsoncpp::value_ref::get_string() const
{
    return get_cstring();
}

int64_t yyjsoncpp::value_ref::get_sint() const
{
    assert(get_type() == type::number);
    return yyjson_get_sint(m_handle);
}

uint64_t yyjsoncpp::value_ref::get_uint() const
{
    assert(get_type() == type::number);
    return yyjson_get_uint(m_handle);
}

double yyjsoncpp::value_ref::get_real() const
{
    assert(get_type() == type::number);
    return yyjson_get_real(m_handle);
}

bool yyjsoncpp::value_ref::get_boolean() const
{
    assert(get_type() == type::boolean);
    return yyjson_get_bool(m_handle);
}

//////////////////////////////////////////////////////////////////////////

yyjsoncpp::mutable_value_ref::mutable_value_ref(mutable_doc* doc, yyjson_mut_val* value) : m_doc(doc), m_handle(value)
{
    assert(m_doc);
    assert(m_handle);
}
