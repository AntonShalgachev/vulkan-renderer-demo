#pragma once

#include <cstdint>
#include <vector>
#include <cassert>

namespace vkgfx
{
    struct ResourceHandle
    {
        std::uint32_t index = static_cast<std::uint32_t>(-1);
        std::uint32_t reincarnation = static_cast<std::uint32_t>(-1);

        auto operator<=>(ResourceHandle const&) const = default;

        operator bool() const { return *this != ResourceHandle{}; }
    };

    // TODO move to common?
    template<typename T>
    class ResourceContainer
    {
    public:
        ResourceHandle add(T value)
        {
            validateInvariants();

            std::size_t objectIndex = m_objects.size();

            if (objectIndex == m_headerIndexMap.size())
            {
                std::size_t newHeaderIndex = m_headers.size();
                m_headerIndexMap.push_back(newHeaderIndex);
                m_headers.push_back(Header{
                    .objectIndex = objectIndex,
                    .reincarnation = static_cast<std::uint32_t>(-1),
                });
            }

            validateInvariants();

            assert(objectIndex < m_headerIndexMap.size());

            std::size_t headerIndex = m_headerIndexMap[objectIndex];

            Header& header = m_headers[headerIndex];

            header.reincarnation++;

            ResourceHandle handle;
            handle.index = headerIndex;
            handle.reincarnation = header.reincarnation;

            m_objects.push_back(std::move(value));

            m_allHandles.push_back(handle);
            m_aliveHandles.push_back(handle);

            validateInvariants();

            return handle;
        }

        void remove(ResourceHandle handle)
        {
            if (!handle)
                return;

            assert(handle.index < m_headers.size());
            Header const& header = m_headers[handle.index];
            if (handle.reincarnation != header.reincarnation)
                return;

            if (header.objectIndex >= m_objects.size())
                return;

            assert(!m_objects.empty());
            swap(header.objectIndex, m_objects.size() - 1);

            assert(header.objectIndex == m_objects.size() - 1);

            validateInvariants();

            m_objects.pop_back();

            m_deadHandles.push_back(handle);
            m_aliveHandles.erase(std::remove(m_aliveHandles.begin(), m_aliveHandles.end(), handle), m_aliveHandles.end());

            validateInvariants();
        }

        T* get(ResourceHandle handle)
        {
            if (!handle)
                return nullptr;

            assert(handle.index < m_headers.size());
            Header const& header = m_headers[handle.index];
            if (handle.reincarnation != header.reincarnation)
                return nullptr;

            if (header.objectIndex >= m_objects.size())
                return nullptr;

            return &m_objects[header.objectIndex];
        }

        T const* get(ResourceHandle handle) const
        {
            if (!handle)
                return nullptr;

            assert(handle.index < m_headers.size());
            Header const& header = m_headers[handle.index];
            if (handle.reincarnation != header.reincarnation)
                return nullptr;

            if (header.objectIndex >= m_objects.size())
                return nullptr;

            return &m_objects[header.objectIndex];
        }

        auto begin() { return m_objects.begin(); }
        auto end() { return m_objects.end(); }
        auto cbegin() const { return m_objects.cbegin(); }
        auto cend() const { return m_objects.cend(); }

    private:
        void swap(std::size_t i1, std::size_t i2)
        {
            validateInvariants();

            if (i1 == i2)
                return;

            std::size_t headerIndex1 = m_headerIndexMap[i1];
            std::size_t headerIndex2 = m_headerIndexMap[i2];

            {
                T o = std::move(m_objects[i2]);
                m_objects[i2] = std::move(m_objects[i1]);
                m_objects[i1] = std::move(o);
            }

            std::swap(m_headers[headerIndex1], m_headers[headerIndex2]);
            std::swap(m_headerIndexMap[i1], m_headerIndexMap[i2]);

            validateInvariants();
        }

        void validateInvariants()
        {
            assert(m_headers.size() == m_headerIndexMap.size());

            for (std::size_t headerIndex = 0; headerIndex < m_headers.size(); headerIndex++)
            {
                std::size_t objectIndex = m_headers[headerIndex].objectIndex;
                assert(m_headerIndexMap[objectIndex] == headerIndex);
            }

            for (std::size_t objectIndex = 0; objectIndex < m_objects.size(); objectIndex++)
            {
                std::size_t headerIndex = m_headerIndexMap[objectIndex];
                assert(m_headers[headerIndex].objectIndex == objectIndex);
            }

            for (ResourceHandle handle : m_aliveHandles)
                assert(get(handle) != nullptr);

            for (ResourceHandle handle : m_deadHandles)
                assert(get(handle) == nullptr);

            for (ResourceHandle handle : m_allHandles)
            {
                T* value = get(handle);
                assert(value == nullptr || value != nullptr);
            }
        }

    private:
        struct Header
        {
            std::size_t objectIndex = static_cast<std::size_t>(-1);
            std::uint32_t reincarnation = static_cast<std::uint32_t>(-1);
        };

        std::vector<Header> m_headers;

        std::vector<T> m_objects;
        std::vector<std::size_t> m_headerIndexMap;

        // for testing
        std::vector<ResourceHandle> m_allHandles;
        std::vector<ResourceHandle> m_aliveHandles;
        std::vector<ResourceHandle> m_deadHandles;
    };
}
