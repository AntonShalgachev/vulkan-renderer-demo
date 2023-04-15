#include "nstl/string_builder.h"

namespace
{
    size_t const minChunkSize = 64;
}

nstl::string_builder& nstl::string_builder::append(string_view str)
{
    string_view charsLeftToCopy = str;

    if (!m_chunks.empty() && m_chunks.back().size() < m_chunks.back().capacity())
    {
        string& lastChunk = m_chunks.back();

        size_t currentCapacity = lastChunk.capacity();

        NSTL_ASSERT(currentCapacity > lastChunk.size());
        size_t availableChars = currentCapacity - lastChunk.size();

        string_view charsToCopyNow = charsLeftToCopy.substr(0, availableChars);
        charsLeftToCopy = charsLeftToCopy.substr(charsToCopyNow.size());

        lastChunk += charsToCopyNow;

        NSTL_ASSERT(lastChunk.capacity() == currentCapacity);
    }

    if (!charsLeftToCopy.empty())
    {
        NSTL_ASSERT(m_chunks.empty() || m_chunks.back().size() == m_chunks.back().capacity());

        string chunk;
        chunk.reserve(nstl::max(charsLeftToCopy.size(), minChunkSize));
        chunk += charsLeftToCopy;
        charsLeftToCopy = {};

        m_chunks.push_back(nstl::move(chunk));
    }

    NSTL_ASSERT(charsLeftToCopy.empty());

    return *this;
}

nstl::string_builder& nstl::string_builder::append(char c)
{
    return append(string_view{ &c, 1 });
}

nstl::string nstl::string_builder::build() const
{
    size_t totalSize = 0;
    for (string const& chunk : m_chunks)
        totalSize += chunk.size();

    string result;
    result.reserve(totalSize);

    [[maybe_unused]] size_t resultCapacity = result.capacity();

    for (string const& chunk : m_chunks)
        result += chunk;

    NSTL_ASSERT(result.capacity() == resultCapacity);

    return result;
}
