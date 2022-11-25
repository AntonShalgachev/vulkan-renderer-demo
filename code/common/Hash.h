#pragma once

#include "nstl/hash.h"

#include <stddef.h>

namespace vkc
{
	namespace hash
	{
		template<typename T>
		void combine(size_t& seed, T const& v)
		{
			nstl::hash<T> hasher;
			seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
		}
	}
}
