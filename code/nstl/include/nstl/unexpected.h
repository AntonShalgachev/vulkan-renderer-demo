#pragma once

#include "type_traits.h"
#include "utility.h"

namespace nstl
{
    template<typename T>
    class Unexpected
    {
        static_assert(!is_void_v<T>, "T shouldn't be void");

    public:
        Unexpected(T value);

        T const& value() const&;
        T& value()&;
        T&& value()&&;

        template<typename U>
        operator Unexpected<U>() const&;

        template<typename U>
        operator Unexpected<U>()&;

        template<typename U>
        operator Unexpected<U>()&&;

    private:
        T m_value;
    };

    template<typename T>
    Unexpected<T> makeUnexpected(T value)
    {
        return Unexpected<remove_cv_t<remove_reference_t<T>>>(nstl::move(value));
    }

    //////////////////////////////////////////////////////////////////////////

    template<typename T>
    nstl::Unexpected<T>::Unexpected(T value) : m_value(nstl::move(value))
    {
    }

    template<typename T>
    T const& nstl::Unexpected<T>::value() const&
    {
        return m_value;
    }

    template<typename T>
    T& nstl::Unexpected<T>::value()&
    {
        return m_value;
    }

    template<typename T>
    T&& nstl::Unexpected<T>::value()&&
    {
        return nstl::move(m_value);
    }

    template<typename T>
    template<typename U>
    nstl::Unexpected<T>::operator Unexpected<U>() const&
    {
        return Unexpected<U>{m_value};
    }

    template<typename T>
    template<typename U>
    nstl::Unexpected<T>::operator Unexpected<U>()&
    {
        return Unexpected<U>{m_value};
    }

    template<typename T>
    template<typename U>
    nstl::Unexpected<T>::operator Unexpected<U>()&&
    {
        return Unexpected<U>{nstl::move(m_value)};
    }
}
