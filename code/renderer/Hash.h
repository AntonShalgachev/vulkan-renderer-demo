#pragma once

#include <cstddef>
#include <functional>

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
