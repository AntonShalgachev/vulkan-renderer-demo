#pragma once

#include "CommandMetadata.h"

#include <string_view>
#include <string>

template<typename CommandsContainer>
class CommandProxy
{
public:
    CommandProxy(CommandsContainer& commands, std::string_view name) : m_commands(commands), m_name(name)
    {

    }

    template<typename Func>
    CommandProxy& operator=(Func func)&&
    {
        m_commands.add(m_name, std::move(m_metadata), std::forward<Func>(func));
        return *this;
    }
    CommandProxy& operator=(std::nullptr_t)
    {
        m_commands.remove(m_name);
        return *this;
    }

    CommandProxy&& description(std::string value)&&
    {
        m_metadata.description = std::move(value);
        return std::move(*this);
    }

    CommandProxy&& arguments(std::vector<std::vector<std::string>> names)&&
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
