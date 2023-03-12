#include "yyjsoncpp/object_ref.h"

#include "yyjsoncpp/array_ref.h"

#include "yyjson.h"

#include <assert.h>

yyjsoncpp::object_iterator::object_iterator(yyjson_val* object)
{
    assert(object);

    m_size = unsafe_yyjson_get_len(object);
    m_index = 0;
    m_handle = m_index < m_size ? unsafe_yyjson_get_first(object) : nullptr;
}

yyjsoncpp::pair yyjsoncpp::object_iterator::operator*() const
{
    value_ref key{ m_handle };
    value_ref value{ m_handle + 1 };
    return { key, value };
}

bool yyjsoncpp::object_iterator::operator==(object_iterator const& rhs) const
{
    return m_handle == rhs.m_handle;
}

yyjsoncpp::object_iterator& yyjsoncpp::object_iterator::operator++()
{
    assert(*this);

    m_index++;
    m_handle = m_index < m_size ? unsafe_yyjson_get_next(m_handle + 1) : nullptr;
    return *this;
}

yyjsoncpp::object_iterator::operator bool() const
{
    assert(m_index <= m_size);
    return m_index < m_size && m_handle;
}

yyjsoncpp::optional<yyjsoncpp::value_ref> yyjsoncpp::object_ref::try_get(string_view name) const
{
    assert(m_handle);
    yyjson_val* val = yyjson_obj_getn(m_handle, name.data(), name.length());
    if (!val)
        return {};

    return value_ref{ val };
}

yyjsoncpp::value_ref yyjsoncpp::object_ref::get(string_view name) const
{
    optional<value_ref> val = try_get(name);
    assert(val);
    return *val;
}

yyjsoncpp::object_iterator yyjsoncpp::object_ref::begin() const
{
    return object_iterator{ m_handle };
}

yyjsoncpp::object_iterator yyjsoncpp::object_ref::end() const
{
    return object_iterator{};
}

//////////////////////////////////////////////////////////////////////////

yyjsoncpp::mutable_object_proxy::mutable_object_proxy(mutable_doc* doc, mutable_object_ref& parent, string_view name)
    : m_doc(doc)
    , m_parent(parent)
    , m_name(name)
{
    assert(m_doc);
}

yyjsoncpp::mutable_object_proxy& yyjsoncpp::mutable_object_proxy::operator=(mutable_value_ref value)
{
    m_parent.add(m_name, value);
    return *this;
}

yyjsoncpp::mutable_object_proxy& yyjsoncpp::mutable_object_proxy::operator=(mutable_array_ref value)
{
    m_parent.add(m_name, value);
    return *this;
}

yyjsoncpp::mutable_object_proxy& yyjsoncpp::mutable_object_proxy::operator=(mutable_object_ref value)
{
    m_parent.add(m_name, value);
    return *this;
}

void yyjsoncpp::mutable_object_proxy::add(mutable_value_ref value)
{
    return m_parent.add(m_name, value);
}

void yyjsoncpp::mutable_object_proxy::add(mutable_array_ref value)
{
    return m_parent.add(m_name, value);
}

void yyjsoncpp::mutable_object_proxy::add(mutable_object_ref value)
{
    return m_parent.add(m_name, value);
}

void yyjsoncpp::mutable_object_ref::add(string_view name, mutable_value_ref value)
{
    assert(m_doc->is_valid());
    mutable_value_ref key = m_doc->create_string(name);
    bool result = yyjson_mut_obj_add(m_handle, key.m_handle, value.m_handle);
    assert(result);
}

void yyjsoncpp::mutable_object_ref::add(string_view name, mutable_array_ref value)
{
    return add(name, static_cast<mutable_value_ref>(value));
}

void yyjsoncpp::mutable_object_ref::add(string_view name, mutable_object_ref value)
{
    return add(name, static_cast<mutable_value_ref>(value));
}

yyjsoncpp::mutable_object_proxy yyjsoncpp::mutable_object_ref::operator[](string_view name)
{
    return mutable_object_proxy{ m_doc, *this, name };
}
