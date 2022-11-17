#pragma once

#include "nstl/string.h"

#include <vector>
#include <sstream>

struct ArgumentMetadata
{
	nstl::string type;
    nstl::string name;
};

struct FunctorMetadata
{
    std::vector<ArgumentMetadata> arguments;
    nstl::string returnType;

    nstl::string buildRepresentation(nstl::string_view name) const // TODO move to cpp
    {
		std::stringstream ss;

		// TODO fix this hack
		auto toStdStringView = [](nstl::string_view sv)
		{
			return std::string_view{ sv.data(), sv.size() };
		};

		if (returnType != "void")
			ss << '[' << toStdStringView(returnType) << "] ";

		ss << toStdStringView(name);

		for (ArgumentMetadata const& argument : arguments)
		{
			nstl::string_view arg = argument.type;
			if (!argument.name.empty())
				arg = argument.name;

			ss << ' ' << toStdStringView(arg);
		}

		// TODO fix this hack
		return { ss.str().c_str() };
    }
};

struct CommandMetadata
{
	nstl::string description;
	std::vector<FunctorMetadata> functors;

    nstl::string type;
};
