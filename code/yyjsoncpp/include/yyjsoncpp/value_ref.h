#pragma once

#include "yyjsoncpp/config.h"
#include "yyjsoncpp/serializer.h"

#include <stdint.h>

struct yyjson_val;
struct yyjson_mut_val;

namespace yyjsoncpp
{
    enum class type;

    class array_ref;
    class object_ref;

    class value_ref
    {
    public:
        explicit value_ref(yyjson_val* value);

        type get_type() const;

        value_ref operator[](string_view name) const;

        template<typename T>
        optional<T> try_get() const
        {
            return serializer<T>::from_json(*this);
        }

        template<typename T>
        T get() const
        {
            optional<T> val = serializer<T>::from_json(*this);
            assert(val);
            return *val;
        }

        array_ref get_array() const;
        object_ref get_object() const;

        char const* get_cstring() const;
        string_view get_string() const;
        int64_t get_sint() const;
        uint64_t get_uint() const;
        double get_real() const;
        bool get_boolean() const;


    protected:
        yyjson_val* m_handle = nullptr;
    };

    //////////////////////////////////////////////////////////////////////////

    class mutable_value_ref
    {
        // TODO remove friends
        friend class mutable_doc;
        friend class mutable_array_ref;
        friend class mutable_object_ref;

    public:
        explicit mutable_value_ref(mutable_doc* doc, yyjson_mut_val* value);

    protected:
        mutable_doc* m_doc = nullptr;
        yyjson_mut_val* m_handle = nullptr;
    };
}
