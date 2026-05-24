#pragma once
#include <glm/glm.hpp>
#include <type_traits>


template<typename T, int L>
struct Bounds
{
	static_assert(L >= 1 && L <= 4);

	using vec_type = glm::vec<L, T, glm::defaultp>;

	vec_type min;
	vec_type max;

	vec_type size() const { return max - min; }

	vec_type center() const
	{
	    return min + (max - min) * static_cast<T>(0.5);
	}

	bool contains(const vec_type& p) const
	{
		bool inside = true;

		for (int i = 0; i < L; ++i)
			inside &= (p[i] >= min[i] && p[i] < max[i]);

		return inside;
	}

	bool intersects(const Bounds& other) const
	{
	    for (int i = 0; i < L; ++i)
	    {
	        if (max[i] <= other.min[i] || min[i] >= other.max[i])
	            return false;
	    }
	    return true;
	}

	void expand(const vec_type& amount)
	{
	    min -= amount;
	    max += amount;
	}
};

#define BOUNDS_DEF(L)                                      \
using IBounds##L    = Bounds<int, L>;                      \
using FBounds##L    = Bounds<float, L>;                    \
using DBounds##L    = Bounds<double, L>;                   \
using UBounds##L    = Bounds<uint32_t, L>;                 \
using I64Bounds##L  = Bounds<int64_t, L>;                  \
using I16Bounds##L  = Bounds<int16_t, L>;                  \
using I8Bounds##L   = Bounds<int8_t, L>;                   \
using U16Bounds##L  = Bounds<uint16_t, L>;                 \
using U8Bounds##L   = Bounds<uint8_t, L>;

BOUNDS_DEF(1);
BOUNDS_DEF(2);
BOUNDS_DEF(3);
BOUNDS_DEF(4);