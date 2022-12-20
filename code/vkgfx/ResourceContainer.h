#pragma once

#include "nstl/vector.h"
#include "nstl/optional.h"
#include "nstl/hash.h"
#include "nstl/utility.h"

#include <cstdint>
#include <assert.h>

// #define VALIDATE_RESOURCE_CONTAINER

// TODO rename this container and move to NSTL

namespace vkgfx
{
    void testResourceContainer(); // TODO remove

    namespace detail
    {
        struct ResourceHeader
        {
            std::size_t objectIndex = static_cast<std::size_t>(-1);
            std::uint32_t reincarnation = static_cast<std::uint32_t>(-1);
        };
    }

    struct ResourceHandle
    {
        std::uint32_t index = static_cast<std::uint32_t>(-1);
        std::uint32_t reincarnation = static_cast<std::uint32_t>(-1);

        auto operator<=>(ResourceHandle const&) const = default;

        explicit operator bool() const { return *this != ResourceHandle{}; }
    };

    template<typename T>
    class ResourceContainer
    {
    public:
        ResourceHandle add(T value)
        {
#ifdef VALIDATE_RESOURCE_CONTAINER
            validateInvariants();
            shuffle();
#endif

            std::size_t objectIndex = m_objects.size();

            if (objectIndex == m_headerIndexMap.size())
            {
                std::size_t newHeaderIndex = m_headers.size();
                m_headerIndexMap.push_back(newHeaderIndex);
                m_headers.push_back(detail::ResourceHeader{
                    .objectIndex = objectIndex,
                    .reincarnation = static_cast<std::uint32_t>(-1),
                });
            }

#ifdef VALIDATE_RESOURCE_CONTAINER
            validateInvariants();
#endif

            assert(objectIndex < m_headerIndexMap.size());

            std::size_t headerIndex = m_headerIndexMap[objectIndex];

            detail::ResourceHeader& header = m_headers[headerIndex];

            header.reincarnation++;

            ResourceHandle handle;
            handle.index = headerIndex;
            handle.reincarnation = header.reincarnation;

            m_objects.push_back(std::move(value));

#ifdef VALIDATE_RESOURCE_CONTAINER
            m_objectHandles.push_back(handle);
            m_allHandles.push_back(handle);
            m_aliveHandles.push_back(handle);
#endif

#ifdef VALIDATE_RESOURCE_CONTAINER
            validateInvariants();
            shuffle();
#endif

            return handle;
        }

        void remove(ResourceHandle handle)
        {
#ifdef VALIDATE_RESOURCE_CONTAINER
            validateInvariants();
            shuffle();
#endif

            if (!handle)
                return;

            assert(handle.index < m_headers.size());
            detail::ResourceHeader const& header = m_headers[handle.index];
            if (handle.reincarnation != header.reincarnation)
                return;

            if (header.objectIndex >= m_objects.size())
                return;

            assert(!m_objects.empty());
            swap(header.objectIndex, m_objects.size() - 1);

            assert(header.objectIndex == m_objects.size() - 1);

#ifdef VALIDATE_RESOURCE_CONTAINER
            validateInvariants();
#endif

            m_objects.pop_back();

#ifdef VALIDATE_RESOURCE_CONTAINER
            m_objectHandles.pop_back();
            m_deadHandles.push_back(handle);
            m_aliveHandles.erase(std::remove(m_aliveHandles.begin(), m_aliveHandles.end(), handle), m_aliveHandles.end());
#endif

#ifdef VALIDATE_RESOURCE_CONTAINER
            validateInvariants();
            shuffle();
#endif
        }

        T* get(ResourceHandle handle)
        {
            if (auto index = getIndex(handle))
                return &m_objects[*index];

            return nullptr;
        }

        T const* get(ResourceHandle handle) const
        {
            if (auto index = getIndex(handle))
                return &m_objects[*index];

            return nullptr;
        }

        auto begin() { return m_objects.begin(); }
        auto end() { return m_objects.end(); }
        auto cbegin() const { return m_objects.cbegin(); }
        auto cend() const { return m_objects.cend(); }

        std::size_t size() const { return m_objects.size(); }
        
        void reserve(std::size_t capacity)
        {
            m_objects.reserve(capacity);
            m_headers.reserve(capacity);
            m_headerIndexMap.reserve(capacity);
        }

    private:
        nstl::optional<std::size_t> getIndex(ResourceHandle handle) const
        {
            if (!handle)
                return {};

            assert(handle.index < m_headers.size());
            detail::ResourceHeader const& header = m_headers[handle.index];
            if (handle.reincarnation != header.reincarnation)
                return {};

            if (header.objectIndex >= m_objects.size())
                return {};

            return header.objectIndex;
        }

        void swap(std::size_t i1, std::size_t i2)
        {
#ifdef VALIDATE_RESOURCE_CONTAINER
            validateInvariants();
#endif

            if (i1 == i2)
                return;

            std::size_t headerIndex1 = m_headerIndexMap[i1];
            std::size_t headerIndex2 = m_headerIndexMap[i2];

            {
                T o = std::move(m_objects[i2]);
                m_objects[i2] = std::move(m_objects[i1]);
                m_objects[i1] = std::move(o);
            }

            nstl::exchange(m_headers[headerIndex1].objectIndex, m_headers[headerIndex2].objectIndex);
            nstl::exchange(m_headerIndexMap[i1], m_headerIndexMap[i2]);

#ifdef VALIDATE_RESOURCE_CONTAINER
            nstl::exchange(m_objectHandles[i1], m_objectHandles[i2]);
#endif

#ifdef VALIDATE_RESOURCE_CONTAINER
            validateInvariants();
#endif
        }

#ifdef VALIDATE_RESOURCE_CONTAINER
        void shuffle()
        {
            for (std::size_t i = 0; i < m_objects.size(); i++)
            {
                std::size_t index0 = m_objects.size() - i - 1;
                std::size_t index1 = std::rand() % (index0 + 1);
                swap(index0, index1);
            }
        }
#endif

#ifdef VALIDATE_RESOURCE_CONTAINER
        void validateInvariants() const
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
                T const* value = get(handle);
                auto index = getIndex(handle);
                assert(value && index || !value && !index);

                if (value && index)
                {
                    assert(&m_objects[*index] == value);
                    assert(m_objectHandles[*index] == handle);
                }
            }
        }
#endif

    private:
        nstl::vector<detail::ResourceHeader> m_headers;

        nstl::vector<T> m_objects;
        nstl::vector<std::size_t> m_headerIndexMap;

#ifdef VALIDATE_RESOURCE_CONTAINER
        nstl::vector<ResourceHandle> m_objectHandles;
        nstl::vector<ResourceHandle> m_allHandles;
        nstl::vector<ResourceHandle> m_aliveHandles;
        nstl::vector<ResourceHandle> m_deadHandles;
#endif
    };
}

template<>
struct nstl::hash<vkgfx::ResourceHandle>
{
    size_t operator()(vkgfx::ResourceHandle const& value) const;
};