#pragma once

#include "yyjsoncpp/serializer.h"
#include "yyjsoncpp/doc.h"
#include "yyjsoncpp/value_ref.h"
#include "yyjsoncpp/type.h"
#include "yyjsoncpp/object_ref.h"

#include "common/tiny_ctti.h"

#include "nstl/sequence.h"

namespace common
{
    template<typename T, typename F, size_t... Is>
    constexpr void for_each_struct_entry_impl(F const& func, nstl::index_sequence<Is...>)
    {
        (func(tiny_ctti::struct_field_v<T, Is>), ...);
    }

    template<typename T, typename F>
    constexpr void for_each_struct_entry(F const& func)
    {
        return for_each_struct_entry_impl<T>(func, nstl::make_index_sequence<tiny_ctti::struct_size_v<T>>{});
    }
}

namespace yyjsoncpp
{
    template<tiny_ctti::described_enum E>
    struct serializer<E>
    {
        static optional<E> from_json(value_ref value)
        {
            if (value.get_type() != type::string)
                return {};

            return tiny_ctti::enum_cast<E>(value.get_string());
        }

        static mutable_value_ref to_json(mutable_doc& doc, E const& mode)
        {
            nstl::string_view name = tiny_ctti::enum_name(mode);
            assert(!name.empty());
            return doc.create_string(name);
        }
    };

    template<tiny_ctti::described_struct T>
    struct serializer<T>
    {
        static optional<T> from_json(value_ref value)
        {
            using StructType = T;

            if (value.get_type() != type::object)
                return {};

            StructType obj{};

            auto json_to_field = [&obj, &value]<typename FieldType>(tiny_ctti::struct_entry<StructType, FieldType> const& entry)
            {
                static_assert(yyjsoncpp::serializable<FieldType>);
                nstl::optional<FieldType> field_value = serializer<FieldType>::from_json(value[entry.name]);
                assert(field_value);
                obj.*(entry.field) = *nstl::move(field_value);
            };

            common::for_each_struct_entry<StructType>(json_to_field);

            return obj;
        }

        static mutable_value_ref to_json(mutable_doc& doc, T const& obj)
        {
            mutable_object_ref root = doc.create_object();

            auto field_to_json = [&obj, &root, &doc]<typename FieldType>(tiny_ctti::struct_entry<T, FieldType> const& entry)
            {
                static_assert(yyjsoncpp::serializable<FieldType>);
                root[entry.name] = serializer<FieldType>::to_json(doc, obj.*(entry.field));
            };

            common::for_each_struct_entry<T>(field_to_json);

            return root;
        }
    };
}
