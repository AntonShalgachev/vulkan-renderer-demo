#pragma once

#include "yyjsoncpp/serializer.h"
#include "yyjsoncpp/doc.h"
#include "yyjsoncpp/value_ref.h"
#include "yyjsoncpp/array_ref.h"
#include "yyjsoncpp/type.h"

#include "nstl/string.h"
#include "nstl/vector.h"
#include "nstl/optional.h"

namespace yyjsoncpp
{
    template<>
    struct serializer<nstl::string>
    {
        static optional<nstl::string> from_json(value_ref obj);
        static mutable_value_ref to_json(mutable_doc& doc, nstl::string const& obj);
    };

    template<typename T>
    struct serializer<nstl::vector<T>>
    {
        static optional<nstl::vector<T>> from_json(value_ref obj)
        {
            if (obj.get_type() != type::array)
                return {};

            nstl::vector<T> result;

            for (value_ref v : obj.get_array())
            {
                optional<T> value = serializer<T>::from_json(v);
                if (!value)
                    return {};

                result.push_back(*nstl::move(value));
            }

            return result;
        }

        static mutable_value_ref to_json(mutable_doc& doc, nstl::vector<T> const& obj)
        {
            mutable_array_ref root = doc.create_array();

            for (T const& value : obj)
                root.push_back(serializer<T>::to_json(doc, value));

            return root;
        }
    };

    template<typename T>
    struct serializer<nstl::optional<T>>
    {
        static optional<nstl::optional<T>> from_json(value_ref obj)
        {
            if (obj.get_type() == yyjsoncpp::type::null)
                return nstl::optional<T>{};

            return serializer<T>::from_json(obj);
        }

        static mutable_value_ref to_json(mutable_doc& doc, nstl::optional<T> const& value)
        {
            if (!value)
                return doc.create_null();

            return serializer<T>::to_json(doc, *value);
        }
    };
}
