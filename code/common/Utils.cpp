#include "Utils.h"

#include <fstream>

nstl::vector<unsigned char> vkc::utils::readFile(const std::string& filename)
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
