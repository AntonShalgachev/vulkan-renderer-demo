#pragma once

#include <cstddef>
#include <type_traits>
#include <vector>

struct VkExtent2D;
struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;

namespace vkr
{
	namespace hash
	{
		template<typename T>
		void combine(std::size_t& seed, T const& v)
		{
			std::hash<T> hasher;
			seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
		}
	}
}

namespace std
{
	template<typename T>
	struct hash<std::vector<T>>
	{
		std::size_t operator()(std::vector<T> const& rhs) const
		{
			std::size_t seed = 0;

			vkr::hash::combine(seed, rhs.size());
			for (T const& v : rhs)
				vkr::hash::combine(seed, v);

			return seed;
		}
	};
}

namespace std
{
	template<>
	struct hash<VkExtent2D>
	{
		std::size_t operator()(VkExtent2D const& rhs) const;
	};
}

namespace std
{
	template<>
	struct hash<VkVertexInputBindingDescription>
	{
		std::size_t operator()(VkVertexInputBindingDescription const& rhs) const;
	};
}

namespace std
{
	template<>
	struct hash<VkVertexInputAttributeDescription>
	{
		std::size_t operator()(VkVertexInputAttributeDescription const& rhs) const;
	};
}
