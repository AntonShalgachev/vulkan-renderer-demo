#pragma once

#include <string>
#include <vector>

struct CommandMetadata
{
    std::string description;
//     std::vector<std::vector<std::string>> positionalParameterTypes;
    std::vector<std::vector<std::string>> positionalParameterNames;
//     std::string type;
};
