#include "yyjsoncpp/doc.h"

#include "yyjsoncpp/value_ref.h"
#include "yyjsoncpp/object_ref.h"
#include "yyjsoncpp/array_ref.h"

#include "yyjson.h"

namespace
{
    yyjsoncpp::read_code convert_read_code(yyjson_read_code code)
    {
        switch (code)
        {
        case YYJSON_READ_SUCCESS: return yyjsoncpp::read_code::success;
        case YYJSON_READ_ERROR_INVALID_PARAMETER: return yyjsoncpp::read_code::error_invalid_parameter;
        case YYJSON_READ_ERROR_MEMORY_ALLOCATION: return yyjsoncpp::read_code::error_memory_allocation;
        case YYJSON_READ_ERROR_EMPTY_CONTENT: return yyjsoncpp::read_code::error_empty_content;
        case YYJSON_READ_ERROR_UNEXPECTED_CONTENT: return yyjsoncpp::read_code::error_unexpected_content;
        case YYJSON_READ_ERROR_UNEXPECTED_END: return yyjsoncpp::read_code::error_unexpected_end;
        case YYJSON_READ_ERROR_UNEXPECTED_CHARACTER: return yyjsoncpp::read_code::error_unexpected_character;
        case YYJSON_READ_ERROR_JSON_STRUCTURE: return yyjsoncpp::read_code::error_json_structure;
        case YYJSON_READ_ERROR_INVALID_COMMENT: return yyjsoncpp::read_code::error_invalid_comment;
        case YYJSON_READ_ERROR_INVALID_NUMBER: return yyjsoncpp::read_code::error_invalid_number;
        case YYJSON_READ_ERROR_INVALID_STRING: return yyjsoncpp::read_code::error_invalid_string;
        case YYJSON_READ_ERROR_LITERAL: return yyjsoncpp::read_code::error_literal;
        case YYJSON_READ_ERROR_FILE_OPEN: return yyjsoncpp::read_code::error_file_open;
        case YYJSON_READ_ERROR_FILE_READ: return yyjsoncpp::read_code::error_file_read;
        }

        assert(false);
        return yyjsoncpp::read_code::unknown;
    }

    class string_allocator
    {
    public:
        string_allocator(yyjsoncpp::string& str) : m_str(str) {}

        static string_allocator& get(void* ctx)
        {
            assert(ctx);
            return *static_cast<string_allocator*>(ctx);
        }

        yyjson_alc get_allocator()
        {
            yyjson_alc allocator;

            allocator.malloc = [](void* ctx, size_t size) { return get(ctx).malloc(size); };
            allocator.realloc = [](void* ctx, void* ptr, size_t old_size, size_t size) { return get(ctx).realloc(ptr, old_size, size); };
            allocator.free = [](void* ctx, void* ptr) { return get(ctx).free(ptr); };
            allocator.ctx = this;

            return allocator;
        }

        void* malloc(size_t size)
        {
            if (m_ptr)
            {
                assert(false);
                return nullptr;
            }

            m_str.resize(size);
            m_ptr = m_str.data();
            return m_ptr;
        }

        void* realloc(void* ptr, size_t, size_t size)
        {
            if (ptr != m_ptr)
            {
                assert(false);
                return nullptr;
            }

            m_str.resize(size);
            m_ptr = m_str.data();
            return m_ptr;
        }

        void free([[maybe_unused]] void* ptr)
        {
            assert(ptr == m_ptr);

            m_str.resize(0);
            m_ptr = nullptr;
        }

    private:
        void* m_ptr = nullptr;
        yyjsoncpp::string& m_str;
    };
}

yyjsoncpp::read_result::operator bool() const
{
    return code == read_code::success;
}

yyjsoncpp::doc::~doc()
{
    yyjson_doc_free(m_handle);
    m_handle = nullptr;
}

yyjsoncpp::read_result yyjsoncpp::doc::read(char const* str, size_t length)
{
    yyjson_read_flag flags = 0;
    yyjson_alc const* allocator = nullptr;

    assert((flags & YYJSON_READ_INSITU) == 0);

    yyjson_read_err error;
    m_handle = yyjson_read_opts(const_cast<char*>(str), length, flags, allocator, &error);

    read_result result;
    result.code = convert_read_code(error.code);
    result.message = error.msg;
    result.position = error.pos;

    if (result)
        assert(is_valid());

    return result;
}

bool yyjsoncpp::doc::is_valid() const
{
    return m_handle != nullptr;
}

yyjsoncpp::value_ref yyjsoncpp::doc::get_root() const
{
    assert(is_valid());

    yyjson_val* value = yyjson_doc_get_root(m_handle);
    assert(value);

    return value_ref{ value };
}

//////////////////////////////////////////////////////////////////////////

yyjsoncpp::mutable_doc::mutable_doc()
{
    yyjson_alc const* allocator = nullptr;
    m_handle = yyjson_mut_doc_new(allocator);
    assert(m_handle);
}

yyjsoncpp::mutable_doc::mutable_doc(doc const& source)
{
    assert(source.is_valid());

    yyjson_alc const* allocator = nullptr;
    m_handle = yyjson_doc_mut_copy(source.m_handle, allocator);
    assert(m_handle);
}

yyjsoncpp::mutable_doc::~mutable_doc()
{
    yyjson_mut_doc_free(m_handle);
    m_handle = nullptr;
}

yyjsoncpp::mutable_array_ref yyjsoncpp::mutable_doc::create_array()
{
    assert(is_valid());
    return mutable_array_ref{ this, yyjson_mut_arr(m_handle) };
}

yyjsoncpp::mutable_object_ref yyjsoncpp::mutable_doc::create_object()
{
    assert(is_valid());
    return mutable_object_ref{ this, yyjson_mut_obj(m_handle) };
}

yyjsoncpp::mutable_value_ref yyjsoncpp::mutable_doc::create_null()
{
    assert(is_valid());
    return mutable_object_ref{ this, yyjson_mut_null(m_handle) };
}

yyjsoncpp::mutable_value_ref yyjsoncpp::mutable_doc::create_string(string_view str)
{
    assert(is_valid());
    return mutable_object_ref{ this, yyjson_mut_strncpy(m_handle, str.data(), str.length()) }; // TODO should we copy a string?
}

yyjsoncpp::mutable_value_ref yyjsoncpp::mutable_doc::create_number(double value)
{
    assert(is_valid());
    return mutable_object_ref{ this, yyjson_mut_real(m_handle, value) };
}

yyjsoncpp::mutable_value_ref yyjsoncpp::mutable_doc::create_boolean(bool value)
{
    assert(is_valid());
    return mutable_object_ref{ this, yyjson_mut_bool(m_handle, value) };
}

yyjsoncpp::mutable_value_ref yyjsoncpp::mutable_doc::create_number(int64_t value)
{
    assert(is_valid());
    return mutable_object_ref{ this, yyjson_mut_sint(m_handle, value) };
}

yyjsoncpp::mutable_value_ref yyjsoncpp::mutable_doc::create_number(uint64_t value)
{
    assert(is_valid());
    return mutable_object_ref{ this, yyjson_mut_uint(m_handle, value) };
}

void yyjsoncpp::mutable_doc::set_root(mutable_value_ref value)
{
    assert(is_valid());
    assert(value.m_handle);
    yyjson_mut_doc_set_root(m_handle, value.m_handle);
}

yyjsoncpp::string yyjsoncpp::mutable_doc::write(write_flags flags)
{
    yyjson_write_flag yyflags = YYJSON_WRITE_NOFLAG;
    if ((flags & write_flags::pretty) == write_flags::pretty)
        yyflags |= YYJSON_WRITE_PRETTY;
    if ((flags & write_flags::escape_unicode) == write_flags::escape_unicode)
        yyflags |= YYJSON_WRITE_ESCAPE_UNICODE;
    if ((flags & write_flags::escsape_slashes) == write_flags::escsape_slashes)
        yyflags |= YYJSON_WRITE_ESCAPE_SLASHES;
    if ((flags & write_flags::allow_inf_and_nan) == write_flags::allow_inf_and_nan)
        yyflags |= YYJSON_WRITE_ALLOW_INF_AND_NAN;
    if ((flags & write_flags::inf_and_nan_as_null) == write_flags::inf_and_nan_as_null)
        yyflags |= YYJSON_WRITE_INF_AND_NAN_AS_NULL;
    if ((flags & write_flags::allow_invalid_unicode) == write_flags::allow_invalid_unicode)
        yyflags |= YYJSON_WRITE_ALLOW_INVALID_UNICODE;

    assert(is_valid());

    string str;
    string_allocator allocator{ str };
    yyjson_alc alc = allocator.get_allocator();

    size_t length = 0;
    yyjson_write_err result;
    [[maybe_unused]] char* str_ptr = yyjson_mut_write_opts(m_handle, yyflags, &alc, &length, &result);
    assert(str_ptr);
    assert(result.code == YYJSON_WRITE_SUCCESS);

    assert(str_ptr == str.data());
    assert(length <= str.size());
    str.resize(length);

    return str;
}

bool yyjsoncpp::mutable_doc::is_valid() const
{
    return m_handle != nullptr;
}

yyjsoncpp::mutable_value_ref yyjsoncpp::mutable_doc::get_root()
{
    assert(is_valid());

    yyjson_mut_val* value = yyjson_mut_doc_get_root(m_handle);
    assert(value);

    return mutable_value_ref{ this, value };
}
