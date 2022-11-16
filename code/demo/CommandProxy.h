#pragma once

#include "CommandMetadata.h"

#include "coil/Coil.h"

#include <string_view>
#include <string>

template<typename CommandsContainer>
class CommandProxy
{
public:
    CommandProxy(CommandsContainer& commands, std::string_view name);

    template<typename Functor>
    CommandProxy& operator=(Functor functor)&&
    {
        // TODO move it somewhere, preferably coil
        static_assert(coil::detail::FuncTraits<Functor>::isFunc, "Func should be a functor object");
        using FunctionWrapper = typename coil::detail::FuncTraits<Functor>::FunctionWrapperType;
        return std::move(*this).operator=(coil::AnyFunctor{ FunctionWrapper{coil::move(functor)} });
    }

    CommandProxy& operator=(coil::AnyFunctor anyFunctor)&&;
    CommandProxy& operator=(coil::Vector<coil::AnyFunctor> anyFunctors)&&;
    CommandProxy& operator=(std::nullptr_t);

    CommandProxy&& description(std::string value)&&;
    CommandProxy&& arguments(std::vector<std::vector<std::string>> names)&&;

    template<typename... T>
    CommandProxy&& arguments(T... names)&&
    {
        return std::move(*this).arguments({ {std::move(names)...} });
    }

private:
    CommandsContainer& m_commands;
    std::string_view m_name;
    CommandMetadata m_metadata;
};

template<typename CommandsContainer>
CommandProxy<CommandsContainer>::CommandProxy(CommandsContainer& commands, std::string_view name) : m_commands(commands), m_name(name)
{

}

template<typename CommandsContainer>
CommandProxy<CommandsContainer>& CommandProxy<CommandsContainer>::operator=(coil::AnyFunctor anyFunctor)&&
{
    coil::Vector<coil::AnyFunctor> functors;
    functors.pushBack(std::move(anyFunctor));
    return std::move(*this).operator=(std::move(functors));
}

template<typename CommandsContainer>
CommandProxy<CommandsContainer>& CommandProxy<CommandsContainer>::operator=(coil::Vector<coil::AnyFunctor> anyFunctors)&&
{
    m_commands.add(m_name, std::move(m_metadata), std::move(anyFunctors));
    return *this;
}

template<typename CommandsContainer>
CommandProxy<CommandsContainer>& CommandProxy<CommandsContainer>::operator=(std::nullptr_t)
{
    m_commands.remove(m_name);
    return *this;
}

template<typename CommandsContainer>
CommandProxy<CommandsContainer>&& CommandProxy<CommandsContainer>::description(std::string value)&&
{
    m_metadata.description = std::move(value);
    return std::move(*this);
}

template<typename CommandsContainer>
CommandProxy<CommandsContainer>&& CommandProxy<CommandsContainer>::arguments(std::vector<std::vector<std::string>> names)&&
{
    if (m_metadata.functors.size() < names.size())
        m_metadata.functors.resize(names.size());

    for (std::size_t i = 0; i < names.size(); i++)
    {
        std::vector<std::string> const& functorArgNames = names[i];
        FunctorMetadata& functorMetadata = m_metadata.functors[i];

        if (functorMetadata.arguments.size() < functorArgNames.size())
            functorMetadata.arguments.resize(functorArgNames.size());

        for (std::size_t j = 0; j < functorArgNames.size(); j++)
        {
            ArgumentMetadata& argumentMetadata = functorMetadata.arguments[j];
            argumentMetadata.name = functorArgNames[j];
        }
    }

    return std::move(*this);
}
