#include "Utils.h"

#include <assert.h>

nstl::vector<unsigned char> vkc::utils::readBinaryFile(char const* filename)
{
    FILE* fp = fopen(filename, "rb");

    assert(fp);

    fseek(fp, 0L, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    nstl::vector<unsigned char> buffer(fileSize);

    size_t readBytes = fread(buffer.data(), 1, buffer.size(), fp);
    assert(readBytes == fileSize);

    fclose(fp);

    return buffer;
}

nstl::string vkc::utils::readTextFile(char const* filename)
{
    FILE* fp = fopen(filename, "rt");

    assert(fp);

    fseek(fp, 0L, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    nstl::string contents;
    contents.resize(fileSize);

    size_t readChars = fread(contents.data(), 1, contents.size(), fp);
    assert(readChars <= fileSize);

    fclose(fp);

    contents.resize(readChars);

    return contents;
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
