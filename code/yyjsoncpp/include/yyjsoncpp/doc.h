#pragma once

#include "yyjsoncpp/config.h"
#include "yyjsoncpp/write_flags.h"
#include "yyjsoncpp/serializer.h"

struct yyjson_doc;
struct yyjson_mut_doc;

namespace yyjsoncpp
{
    class value_ref;

    enum class read_code
    {
        unknown,
        success,
        error_invalid_parameter,
        error_memory_allocation,
        error_empty_content,
        error_unexpected_content,
        error_unexpected_end,
        error_unexpected_character,
        error_json_structure,
        error_invalid_comment,
        error_invalid_number,
        error_invalid_string,
        error_literal,
        error_file_open,
        error_file_read,
    };

    struct read_result
    {
        operator bool() const;

        read_code code = read_code::unknown;
        string_view message = {};
        size_t position = 0;
    };

    class doc
    {
        friend class mutable_doc;

    public:
        doc() = default;
        // TODO move/copy constructors and assignment operators
        ~doc();

        read_result read(char const* str, size_t length);

        //void set_allocator(nullptr_t)
        //{
        //    // TODO
        //}

        bool is_valid() const;

        value_ref get_root() const;

    private:
        yyjson_doc* m_handle = nullptr;
    };

    //////////////////////////////////////////////////////////////////////////

    class mutable_value_ref;
    class mutable_array_ref;
    class mutable_object_ref;

    class mutable_doc
    {
        friend class doc;
        friend class mutable_value_ref;
        friend class mutable_array_ref;
        friend class mutable_object_ref;

    public:
        // TODO add allocator
        mutable_doc();
        explicit mutable_doc(doc const& source);
        // TODO move/copy constructors and assignment operators
        ~mutable_doc();

        mutable_array_ref create_array();
        mutable_object_ref create_object();

        mutable_value_ref create_null();
        mutable_value_ref create_string(string_view const& str);

        template<typename T>
        mutable_value_ref create_value(T const& value)
        {
            return serializer<T>::to_json(*this, value);
        }

        void set_root(mutable_value_ref value);

        string write(write_flags flags = write_flags{});

//         void set_allocator(nullptr_t)
//         {
//             // TODO
//         }

        bool is_valid() const;

        mutable_value_ref get_root();

    private:
        yyjson_mut_doc* m_handle = nullptr;
    };
}
