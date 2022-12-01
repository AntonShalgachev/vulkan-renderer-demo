#include "Utils.h"

#include <fstream>

nstl::vector<unsigned char> vkc::utils::readFile(char const* filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error("failed to open file!");

    std::streamsize fileSize = file.tellg();
    nstl::vector<unsigned char> buffer(static_cast<std::size_t>(fileSize));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize); // safe, since char and unsigned char have the same alignment and representation
    file.close();

    return buffer;
}

nstl::string vkc::utils::readTextFile(char const* filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
        throw std::runtime_error("failed to open file!");

    file.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize fileSize = file.gcount();
    file.clear();

    nstl::string content;
    content.resize(static_cast<std::size_t>(fileSize));

    file.seekg(0);
    file.read(content.data(), content.size());
    file.close();

    return content;
}

nstl::vector<nstl::string_view> vkc::utils::split(nstl::string_view str)
{
    nstl::vector<nstl::string_view> result;
    nstl::string_view rest = str;
    
    while (!rest.empty())
    {
        auto pos = rest.find('\n');
        result.push_back(rest.substr(0, pos));

        if (pos != nstl::string_view::npos)
            rest = rest.substr(pos + 1);
        else
            rest = {};
    }

    return result;
}
