#pragma once

#include "assert.h"
#include "hash.h"
#include "vector.h"
#include "utility.h"

namespace nstl
{
    template<typename K, typename V>
    class key_value_pair
    {
        template<typename, typename>
        friend class unordered_map;

    public:
        K const& key() const
        {
            return m_key;
        }
        V const& value() const
        {
            return m_value;
        }
        V& value()
        {
            return m_value;
        }

    private:
        key_value_pair(K key, V value) : m_key(nstl::move(key)), m_value(nstl::move(value)) {}

        K m_key;
        V m_value;
    };

    template<typename K, typename V>
    class unordered_map
    {
    public:
        struct node
        {
            key_value_pair<K, V> pair;

            size_t bucket = invalidIndex;
            size_t prev = invalidIndex;
            size_t next = invalidIndex;
        };

        class iterator
        {
        public:
            iterator(node* node) : m_node(node) {}

            key_value_pair<K, V>& operator*()
            {
                return m_node->pair;
            }
            key_value_pair<K, V>* operator->()
            {
                return &m_node->pair;
            }

            bool operator==(iterator const& rhs) const
            {
                return m_node == rhs.m_node;
            }

            bool operator!=(iterator const& rhs) const
            {
                return !(*this == rhs);
            }

            iterator& operator++()
            {
                m_node++;
                return *this;
            }

        private:
            node* m_node = nullptr;
        };

        class const_iterator
        {
        public:
            const_iterator(node const* node) : m_node(node) {}

            key_value_pair<K, V> const& operator*() const
            {
                return m_node->pair;
            }
            key_value_pair<K, V> const* operator->() const
            {
                return &m_node->pair;
            }

            bool operator==(const_iterator const& rhs) const
            {
                return m_node == rhs.m_node;
            }

            bool operator!=(const_iterator const& rhs) const
            {
                return !(*this == rhs);
            }

            const_iterator& operator++()
            {
                m_node++;
                return *this;
            }

        private:
            node const* m_node = nullptr;
        };

        unordered_map(size_t bucketsCount = 1, float maxLoadFactor = 1.0f) : m_maxLoadFactor(maxLoadFactor)
        {
            rehash(bucketsCount);
        }

        const_iterator begin() const
        {
            return { m_nodes.begin() };
        }
        const_iterator end() const
        {
            return { m_nodes.end() };
        }
        iterator begin()
        {
            return { m_nodes.begin() };
        }
        iterator end()
        {
            return { m_nodes.end() };
        }

        iterator insert_or_assign(K key, V value)
        {
            if (iterator it = find(key); it != end())
            {
                it->m_value = nstl::move(value);
                return it;
            }

            size_t nextSize = size() + 1;
            float nextLoadFactor = 1.0f * nextSize / m_buckets.size();
            if (nextLoadFactor > m_maxLoadFactor)
                rehash(size() * 2);

            size_t newNodeIndex = m_nodes.size();
            m_nodes.push_back({ key_value_pair<K, V>{nstl::move(key), nstl::move(value)}, invalidIndex, invalidIndex, invalidIndex });

            insert_node(newNodeIndex);

            return iterator{ &m_nodes[newNodeIndex] };
        }

        template<typename T>
        void erase(T const& key)
        {
            size_t nodeIndex = find_node_index(key);
            if (nodeIndex == invalidIndex)
                return;

            NSTL_ASSERT(nodeIndex < m_nodes.size());
            node& erasedNode = m_nodes[nodeIndex];

            NSTL_ASSERT(erasedNode.bucket < m_buckets.size());

            if (erasedNode.prev != invalidIndex)
            {
                NSTL_ASSERT(m_buckets[erasedNode.bucket] != nodeIndex);
                NSTL_ASSERT(erasedNode.prev < m_nodes.size());
                m_nodes[erasedNode.prev].next = erasedNode.next;
            }
            else
            {
                NSTL_ASSERT(m_buckets[erasedNode.bucket] == nodeIndex);
                m_buckets[erasedNode.bucket] = erasedNode.next;
            }

            if (erasedNode.next != invalidIndex)
            {
                NSTL_ASSERT(erasedNode.next < m_nodes.size());
                m_nodes[erasedNode.next].prev = erasedNode.prev;
            }

            erasedNode.prev = invalidIndex;
            erasedNode.next = invalidIndex;

            NSTL_ASSERT(!m_nodes.empty());

            if (nodeIndex != m_nodes.size() - 1)
            {
                node& lastNode = m_nodes.back();

                if (lastNode.prev != invalidIndex)
                {
                    NSTL_ASSERT(lastNode.prev < m_nodes.size());
                    m_nodes[lastNode.prev].next = nodeIndex;
                }
                else
                {
                    NSTL_ASSERT(lastNode.bucket < m_buckets.size());
                    m_buckets[lastNode.bucket] = nodeIndex;
                }

                nstl::exchange(erasedNode, lastNode);
            }

            NSTL_ASSERT(m_nodes.back().prev == invalidIndex);
            NSTL_ASSERT(m_nodes.back().next == invalidIndex);

            m_nodes.pop_back();
        }

        void clear()
        {
            m_nodes.clear();
            reset_buckets();
        }

        template<typename T>
        iterator find(T const& key)
        {
            size_t index = find_node_index(key);
            if (index != invalidIndex)
                return iterator{&m_nodes[index]};
            return end();
        }

        template<typename T>
        const_iterator find(T const& key) const
        {
            size_t index = find_node_index(key);
            if (index != invalidIndex)
                return const_iterator{&m_nodes[index]};
            return end();
        }

        size_t size() const
        {
            return m_nodes.size();
        }

    private:
        template<typename T>
        size_t compute_bucket_index(T const& key) const
        {
            size_t hash = nstl::hash<T>{}(key);
            return hash % m_buckets.size();
        }

        void insert_node(size_t newNodeIndex)
        {
            NSTL_ASSERT(newNodeIndex < m_nodes.size());
            node& newNode = m_nodes[newNodeIndex];

            size_t bucketIndex = compute_bucket_index(newNode.pair.m_key);
            NSTL_ASSERT(bucketIndex < m_buckets.size());

            newNode.bucket = bucketIndex;
            newNode.next = invalidIndex;
            newNode.prev = invalidIndex;

            size_t firstNodeIndex = m_buckets[bucketIndex];
            if (firstNodeIndex == invalidIndex)
            {
                m_buckets[bucketIndex] = newNodeIndex;
                return;
            }

            size_t nodeIndex = firstNodeIndex;
            while (nodeIndex != invalidIndex)
            {
                NSTL_ASSERT(nodeIndex < m_nodes.size());
                node& node = m_nodes[nodeIndex];

                if (node.next == invalidIndex)
                {
                    node.next = newNodeIndex;
                    newNode.prev = nodeIndex;
                    return;
                }

                nodeIndex = node.next;
            }
        }

        void reset_buckets()
        {
            for (size_t i = 0; i < m_buckets.size(); i++)
                m_buckets[i] = invalidIndex;
        }

        void rehash(size_t buckets)
        {
            m_buckets.resize(buckets);
            reset_buckets();

            for (size_t nodeIndex = 0; nodeIndex < m_nodes.size(); nodeIndex++)
                insert_node(nodeIndex);
        }

        template<typename T>
        size_t find_node_index(T const& key) const
        {
            size_t bucketIndex = compute_bucket_index(key);
            NSTL_ASSERT(bucketIndex < m_buckets.size());

            size_t nodeIndex = m_buckets[bucketIndex];
            while (nodeIndex != invalidIndex)
            {
                NSTL_ASSERT(nodeIndex < m_nodes.size());
                node const& node = m_nodes[nodeIndex];

                if (node.pair.m_key == key)
                    return nodeIndex;

                nodeIndex = node.next;
            }

            return invalidIndex;
        }

    private:
        inline static constexpr size_t invalidIndex = static_cast<size_t>(-1);

    private:
        vector<size_t> m_buckets;
        vector<node> m_nodes;
        float m_maxLoadFactor = 1.0f;
    };
}
