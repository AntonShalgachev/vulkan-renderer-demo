#pragma once

#include "yyjsoncpp/value_ref.h"

#include "yyjsoncpp/doc.h"

namespace yyjsoncpp
{
    class array_iterator
    {
    public:
        explicit array_iterator() = default;
        explicit array_iterator(yyjson_val* array);

        value_ref operator*() const;
        bool operator==(array_iterator const&) const;
        array_iterator& operator++();
        array_iterator operator++(int);
        operator bool() const;

        size_t index() const;

    private:
        size_t m_size = 0;
        size_t m_index = 0;
        yyjson_val* m_handle = nullptr;
    };

    class array_ref : public value_ref
    {
    public:
        using value_ref::value_ref;

        array_iterator begin() const;
        array_iterator end() const;

        size_t size() const;
    };

    //////////////////////////////////////////////////////////////////////////

    class mutable_object_ref;

    class mutable_array_ref : public mutable_value_ref
    {
    public:
        using mutable_value_ref::mutable_value_ref;

        template<typename T>
        void push_back(T&& value)
        {
            assert(m_doc);
            return push_back(m_doc->create_value(value));
        }
        void push_back(mutable_value_ref value);
        void push_back(mutable_array_ref value);
        void push_back(mutable_object_ref value);
    };
}
