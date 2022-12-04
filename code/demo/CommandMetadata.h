#pragma once

#include "nstl/string.h"
#include "nstl/string_view.h"
#include "nstl/vector.h"

struct ArgumentMetadata
{
	nstl::string type;
    nstl::string name;
};

struct FunctorMetadata
{
    nstl::vector<ArgumentMetadata> arguments;
    nstl::string returnType;

    nstl::string buildRepresentation(nstl::string_view name) const;
};

struct CommandMetadata
{
	nstl::string description;
	nstl::vector<FunctorMetadata> functors;

    nstl::string type;
};
