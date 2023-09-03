#pragma once

#include "yyjsoncpp/value_ref.h"

#include "yyjsoncpp/doc.h"

namespace yyjsoncpp
{
    struct pair
    {
        value_ref key;
        value_ref value;
    };

    class object_iterator
    {
    public:
        explicit object_iterator() = default;
        explicit object_iterator(yyjson_val* object);

        pair operator*() const;

        bool operator==(object_iterator const&) const;

        object_iterator& operator++();

        explicit operator bool() const;

    private:
        size_t m_size = 0;
        size_t m_index = 0;
        yyjson_val* m_handle = nullptr;
    };

    class object_ref : public value_ref
    {
    public:
        using value_ref::value_ref;

        optional<value_ref> try_get(string_view name) const;
        value_ref get(string_view name) const;

        object_iterator begin() const;
        object_iterator end() const;
    };

    //////////////////////////////////////////////////////////////////////////

    class mutable_doc;
    class mutable_array_ref;
    class mutable_object_ref;

    class mutable_object_proxy
    {
    public:
        mutable_object_proxy(mutable_doc* doc, mutable_object_ref& parent, string_view name);

        template<typename T>
        mutable_object_proxy& operator=(T const& value)
        {
            assert(m_doc);
            add(m_doc->create_value(value));
            return *this;
        }
        mutable_object_proxy& operator=(mutable_value_ref value);
        mutable_object_proxy& operator=(mutable_array_ref value);
        mutable_object_proxy& operator=(mutable_object_ref value);

        template<typename T>
        void add(T const& value)
        {
            assert(m_doc);
            return add(m_doc->create_value(value));
        }
        void add(mutable_value_ref value);
        void add(mutable_array_ref value);
        void add(mutable_object_ref value);

    private:
        mutable_doc* m_doc = nullptr;
        mutable_object_ref& m_parent;
        string_view m_name;
    };

    class mutable_object_ref : public mutable_value_ref
    {
    public:
        using mutable_value_ref::mutable_value_ref;

        template<typename T>
        void add(string_view name, T const& value)
        {
            assert(m_doc);
            return add(name, m_doc->create_value(value));
        }
        void add(string_view name, mutable_value_ref value);
        void add(string_view name, mutable_array_ref value);
        void add(string_view name, mutable_object_ref value);

        mutable_object_proxy operator[](string_view name);
    };
}
