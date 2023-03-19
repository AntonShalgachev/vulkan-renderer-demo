#include "yyjsoncpp/default_serializer.h"

#include "yyjsoncpp/value_ref.h"
#include "yyjsoncpp/doc.h"
#include "yyjsoncpp/type.h"

// TODO check bounds for the arithmetic types

namespace
{
    template<typename T>
    yyjsoncpp::mutable_value_ref sint_to_json(yyjsoncpp::mutable_doc& doc, T const& value)
    {
        return doc.create_number(static_cast<int64_t>(value));
    }

    template<typename T>
    yyjsoncpp::mutable_value_ref uint_to_json(yyjsoncpp::mutable_doc& doc, T const& value)
    {
        return doc.create_number(static_cast<uint64_t>(value));
    }

    template<typename T>
    yyjsoncpp::mutable_value_ref real_to_json(yyjsoncpp::mutable_doc& doc, T const& value)
    {
        return doc.create_number(static_cast<double>(value));
    }

    //////////////////////////////////////////////////////////////////////////

    template<typename T>
    yyjsoncpp::optional<T> sint_from_json(yyjsoncpp::value_ref value)
    {
        if (value.get_type() != yyjsoncpp::type::number)
            return {};

        return value.get_sint();
    }

    template<typename T>
    yyjsoncpp::optional<T> uint_from_json(yyjsoncpp::value_ref value)
    {
        if (value.get_type() != yyjsoncpp::type::number)
            return {};

        return value.get_uint();
    }

    template<typename T>
    yyjsoncpp::optional<T> real_from_json(yyjsoncpp::value_ref value)
    {
        if (value.get_type() != yyjsoncpp::type::number)
            return {};

        return value.get_real();
    }

    //////////////////////////////////////////////////////////////////////////

    template<typename T> auto to_json_func = nullptr;
    template<> auto to_json_func<char> = &sint_to_json<char>;
    template<> auto to_json_func<signed char> = &sint_to_json<signed char>;
    template<> auto to_json_func<short> = &sint_to_json<short>;
    template<> auto to_json_func<int> = &sint_to_json<int>;
    template<> auto to_json_func<long> = &sint_to_json<long>;
    template<> auto to_json_func<long long> = &sint_to_json<long long>;

    template<> auto to_json_func<unsigned char> = &uint_to_json<unsigned char>;
    template<> auto to_json_func<unsigned short> = &uint_to_json<unsigned short>;
    template<> auto to_json_func<unsigned int> = &uint_to_json<unsigned int>;
    template<> auto to_json_func<unsigned long> = &uint_to_json<unsigned long>;
    template<> auto to_json_func<unsigned long long> = &uint_to_json<unsigned long long>;

    template<> auto to_json_func<float> = &real_to_json<float>;
    template<> auto to_json_func<double> = &real_to_json<double>;
    template<> auto to_json_func<long double> = &real_to_json<long double>;

    template<typename T> auto from_json_func = nullptr;
    template<> auto from_json_func<char> = &sint_from_json<char>;
    template<> auto from_json_func<signed char> = &sint_from_json<signed char>;
    template<> auto from_json_func<short> = &sint_from_json<short>;
    template<> auto from_json_func<int> = &sint_from_json<int>;
    template<> auto from_json_func<long> = &sint_from_json<long>;
    template<> auto from_json_func<long long> = &sint_from_json<long long>;

    template<> auto from_json_func<unsigned char> = &uint_from_json<unsigned char>;
    template<> auto from_json_func<unsigned short> = &uint_from_json<unsigned short>;
    template<> auto from_json_func<unsigned int> = &uint_from_json<unsigned int>;
    template<> auto from_json_func<unsigned long> = &uint_from_json<unsigned long>;
    template<> auto from_json_func<unsigned long long> = &uint_from_json<unsigned long long>;

    template<> auto from_json_func<float> = &real_from_json<float>;
    template<> auto from_json_func<double> = &real_from_json<double>;
    template<> auto from_json_func<long double> = &real_from_json<long double>;
}

yyjsoncpp::optional<char const*> yyjsoncpp::serializer<char const*>::from_json(value_ref value)
{
    if (value.get_type() != type::string)
        return {};

    return value.get_cstring();
}

yyjsoncpp::mutable_value_ref yyjsoncpp::serializer<char const*>::to_json(mutable_doc& doc, char const* const& value)
{
    return doc.create_string(value);
}

yyjsoncpp::optional<nstl::string_view> yyjsoncpp::serializer<nstl::string_view>::from_json(value_ref value)
{
    if (value.get_type() != type::string)
        return {};

    return value.get_string();
}

yyjsoncpp::mutable_value_ref yyjsoncpp::serializer<nstl::string_view>::to_json(mutable_doc& doc, nstl::string_view const& value)
{
    return doc.create_string(value);
}

#define YYJSONCPP_IMPLEMENT_ARITHMETIC_SERIALIZER(T)                                                                                      \
yyjsoncpp::optional<T> yyjsoncpp::serializer<T>::from_json(value_ref value) { return from_json_func<T>(value); }                          \
yyjsoncpp::mutable_value_ref yyjsoncpp::serializer<T>::to_json(mutable_doc& doc, T const& value) { return to_json_func<T>(doc, value); }

YYJSONCPP_IMPLEMENT_ARITHMETIC_SERIALIZER(char);
YYJSONCPP_IMPLEMENT_ARITHMETIC_SERIALIZER(signed char);
YYJSONCPP_IMPLEMENT_ARITHMETIC_SERIALIZER(unsigned char);
YYJSONCPP_IMPLEMENT_ARITHMETIC_SERIALIZER(short);
YYJSONCPP_IMPLEMENT_ARITHMETIC_SERIALIZER(unsigned short);
YYJSONCPP_IMPLEMENT_ARITHMETIC_SERIALIZER(int);
YYJSONCPP_IMPLEMENT_ARITHMETIC_SERIALIZER(unsigned int);
YYJSONCPP_IMPLEMENT_ARITHMETIC_SERIALIZER(long);
YYJSONCPP_IMPLEMENT_ARITHMETIC_SERIALIZER(unsigned long);
YYJSONCPP_IMPLEMENT_ARITHMETIC_SERIALIZER(long long);
YYJSONCPP_IMPLEMENT_ARITHMETIC_SERIALIZER(unsigned long long);
YYJSONCPP_IMPLEMENT_ARITHMETIC_SERIALIZER(float);
YYJSONCPP_IMPLEMENT_ARITHMETIC_SERIALIZER(double);
YYJSONCPP_IMPLEMENT_ARITHMETIC_SERIALIZER(long double);

#undef YYJSONCPP_IMPLEMENT_ARITHMETIC_SERIALIZER

yyjsoncpp::optional<bool> yyjsoncpp::serializer<bool>::from_json(value_ref value)
{
    if (value.get_type() != type::boolean)
        return {};

    return value.get_boolean();
}

yyjsoncpp::mutable_value_ref yyjsoncpp::serializer<bool>::to_json(mutable_doc& doc, bool const& value)
{
    return doc.create_boolean(value);
}
