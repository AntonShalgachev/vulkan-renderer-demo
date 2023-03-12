#include "yyjsoncpp/array_ref.h"

#include "yyjsoncpp/object_ref.h"

#include "yyjson.h"

#include <assert.h>

yyjsoncpp::array_iterator::array_iterator(yyjson_val* array)
{
    assert(array);

    m_size = unsafe_yyjson_get_len(array);
    m_index = 0;
    m_handle = m_index < m_size ? unsafe_yyjson_get_first(array) : nullptr;
}

yyjsoncpp::value_ref yyjsoncpp::array_iterator::operator*() const
{
    assert(*this);

    return value_ref{ m_handle };
}

bool yyjsoncpp::array_iterator::operator==(array_iterator const& rhs) const
{
    return m_handle == rhs.m_handle;
}

yyjsoncpp::array_iterator& yyjsoncpp::array_iterator::operator++()
{
    assert(*this);

    m_index++;
    m_handle = m_index < m_size ? unsafe_yyjson_get_next(m_handle) : nullptr;
    return *this;
}

yyjsoncpp::array_iterator::operator bool() const
{
    assert(m_index <= m_size);
    return m_index < m_size && m_handle;
}

yyjsoncpp::array_iterator yyjsoncpp::array_ref::begin() const
{
    return array_iterator{ m_handle };
}

yyjsoncpp::array_iterator yyjsoncpp::array_ref::end() const
{
    return array_iterator{};
}

//////////////////////////////////////////////////////////////////////////

void yyjsoncpp::mutable_array_ref::push_back(mutable_value_ref value)
{
    bool result = yyjson_mut_arr_append(m_handle, value.m_handle);
    assert(result);
}

void yyjsoncpp::mutable_array_ref::push_back(mutable_array_ref value)
{
    return push_back(static_cast<mutable_value_ref>(value));
}

void yyjsoncpp::mutable_array_ref::push_back(mutable_object_ref value)
{
    return push_back(static_cast<mutable_value_ref>(value));
}
