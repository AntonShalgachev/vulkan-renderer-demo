#pragma once

#include "CommandMetadata.h"

#include "coil/Coil.h"

#include "nstl/string_view.h"
#include "nstl/string.h"
#include "nstl/vector.h"

template<typename CommandsContainer>
class CommandProxy
{
public:
    CommandProxy(CommandsContainer& commands, nstl::string_view name);

    template<typename Functor>
    CommandProxy& operator=(Functor functor)&&
    {
        // TODO move it somewhere, preferably coil
        static_assert(coil::detail::FuncTraits<Functor>::isFunc, "Func should be a functor object");
        using FunctionWrapper = typename coil::detail::FuncTraits<Functor>::FunctionWrapperType;
        return nstl::move(*this).operator=(coil::AnyFunctor{ FunctionWrapper{coil::move(functor)} });
    }

    CommandProxy& operator=(coil::AnyFunctor anyFunctor)&&;
    CommandProxy& operator=(coil::Vector<coil::AnyFunctor> anyFunctors)&&;
    CommandProxy& operator=(nullptr_t);

    CommandProxy&& description(nstl::string value)&&;
    CommandProxy&& arguments(nstl::vector<nstl::vector<nstl::string>> names)&&;

    template<typename... T>
    CommandProxy&& arguments(T... names)&&
    {
        return nstl::move(*this).arguments({ {nstl::move(names)...} });
    }

private:
    CommandsContainer& m_commands;
    nstl::string_view m_name;
    CommandMetadata m_metadata;
};

template<typename CommandsContainer>
CommandProxy<CommandsContainer>::CommandProxy(CommandsContainer& commands, nstl::string_view name) : m_commands(commands), m_name(name)
{

}

template<typename CommandsContainer>
CommandProxy<CommandsContainer>& CommandProxy<CommandsContainer>::operator=(coil::AnyFunctor anyFunctor)&&
{
    coil::Vector<coil::AnyFunctor> functors;
    functors.pushBack(nstl::move(anyFunctor));
    return nstl::move(*this).operator=(nstl::move(functors));
}

template<typename CommandsContainer>
CommandProxy<CommandsContainer>& CommandProxy<CommandsContainer>::operator=(coil::Vector<coil::AnyFunctor> anyFunctors)&&
{
    m_commands.add(m_name, nstl::move(m_metadata), nstl::move(anyFunctors));
    return *this;
}

template<typename CommandsContainer>
CommandProxy<CommandsContainer>& CommandProxy<CommandsContainer>::operator=(nullptr_t)
{
    m_commands.remove(m_name);
    return *this;
}

template<typename CommandsContainer>
CommandProxy<CommandsContainer>&& CommandProxy<CommandsContainer>::description(nstl::string value)&&
{
    m_metadata.description = nstl::move(value);
    return nstl::move(*this);
}

template<typename CommandsContainer>
CommandProxy<CommandsContainer>&& CommandProxy<CommandsContainer>::arguments(nstl::vector<nstl::vector<nstl::string>> names)&&
{
    if (m_metadata.functors.size() < names.size())
        m_metadata.functors.resize(names.size());

    for (size_t i = 0; i < names.size(); i++)
    {
        nstl::vector<nstl::string> const& functorArgNames = names[i];
        FunctorMetadata& functorMetadata = m_metadata.functors[i];

        if (functorMetadata.arguments.size() < functorArgNames.size())
            functorMetadata.arguments.resize(functorArgNames.size());

        for (size_t j = 0; j < functorArgNames.size(); j++)
        {
            ArgumentMetadata& argumentMetadata = functorMetadata.arguments[j];
            argumentMetadata.name = functorArgNames[j];
        }
    }

    return nstl::move(*this);
}
