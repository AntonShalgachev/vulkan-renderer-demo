#pragma once

#include <string>
#include <vector>
#include <sstream>

struct ArgumentMetadata
{
	std::string type;
    std::string name;
};

struct FunctorMetadata
{
    std::vector<ArgumentMetadata> arguments;
    std::string returnType;

    std::string buildRepresentation(std::string_view name) const // TODO move to cpp
    {
		std::stringstream ss;

		if (!returnType.empty() && returnType != "void")
			ss << '[' << returnType << "] ";

		ss << name;

		for (ArgumentMetadata const& argument : arguments)
		{
			std::string_view arg = argument.type;
			if (!argument.name.empty())
				arg = argument.name;

			ss << ' ' << arg;
		}

		return ss.str();
    }
};

struct CommandMetadata
{
	std::string description;
	std::vector<FunctorMetadata> functors;

    std::string type;
};
