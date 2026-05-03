#pragma once

#include <glm/glm.hpp>
#include <BitPounce/Core/Timer.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace BitPounce::Math {


	
	glm::mat4 ComposeTransform(const glm::vec3 &translation, const glm::vec3 &rotation, const glm::vec3 &scale);

	template<typename T>
	constexpr T GetPI()
	{
		return static_cast<T>(3.141592653589793238462643383279502884L);
	}

	template<typename T>
	constexpr T Abs(T x)
	{
		return x < T(0) ? -x : x;
	}
	

	template<typename T>
	constexpr T WrapPI(T x)
	{
		const T PI = GetPI<T>();
		const T TWO_PI = PI * T(2);

		while (x > PI)  x -= TWO_PI;
		while (x < -PI) x += TWO_PI;

		return x;
	}
	template<typename T>
	constexpr T Sqrt(T x)
	{
		if (x <= T(0)) return T(0);

		T guess = x;
		for (int i = 0; i < 5; i++)
			guess = T(0.5) * (guess + x / guess);

		return guess;
	}

	template<typename T>
	constexpr T Sin(T x)
	{
		const T PI = GetPI<T>();
		const T P = T(0.225);

		x = WrapPI(x);

		const T B = T(4) / PI;
		const T C = -T(4) / (PI * PI);

		T y = B * x + C * x * Abs(x);

		y = P * (y * Abs(y) - y) + y;

		return y;
	}

	template<typename T>
	constexpr T Cos(T x)
	{
		const T HALF_PI = GetPI<T>() / T(2);
		return Sin(x + HALF_PI);
	}

	template<typename T>
	constexpr T Atan2(T y, T x)
	{
		const T PI = GetPI<T>();
		const T HALF_PI = PI / T(2);
		const T PI_4 = PI / T(4);
		const T THREE_PI_4 = PI * T(3) / T(4);

		if (x == T(0)) {
			if (y > T(0)) return HALF_PI;
			if (y < T(0)) return -HALF_PI;
			return T(0);
		}

		T abs_y = Abs(y);
		T angle;

		if (x > T(0)) {
			T r = (x - abs_y) / (x + abs_y);
			angle = PI_4 - PI_4 * r;
		}
		else {
			T r = (x + abs_y) / (abs_y - x);
			angle = THREE_PI_4 - PI_4 * r;
		}
		return (y < T(0)) ? -angle : angle;
	}

	template<typename T>
	constexpr T Asin(T x)
	{
		const T HALF_PI = GetPI<T>() / T(2);

		// Clamp
		if (x > T(1))  x = T(1);
		if (x < T(-1)) x = T(-1);

		T sign = (x < T(0)) ? T(-1) : T(1);
		x = Abs(x);

		// Polynomial (minimax-style)
		T p = T(-0.0187293);
		p = p * x + T(0.0742610);
		p = p * x - T(0.2121144);
		p = p * x + T(1.5707288);

		T result = HALF_PI - Sqrt(T(1) - x) * p;

		return sign * result;
	}

	template<typename T>
	constexpr T dot(const glm::vec3& a, const glm::vec3& b) {
	        return a.x * b.x + a.y * b.y + a.z * b.z;
	}
	template<typename T>
	constexpr T length(const glm::vec3& v) {
	        return Sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	    }
	template<typename T>
	constexpr T normalize(const glm::vec3& v) {
	        T len = length(v);
			constexpr T EPS = T(1e-8);
	        return len > EPS ? v / len : v;
	    }
	template<typename T>
	constexpr glm::vec3 cross(const glm::vec3& a, const glm::vec3& b) {
	        return glm::vec3(a.y * b.z - a.z * b.y,
	                    a.z * b.x - a.x * b.z,
	                    a.x * b.y - a.y * b.x);
	    }

	inline bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
		{
		    // From glm::decompose in matrix_decompose.inl

			using namespace glm;
			using T = float;

			mat4 LocalMatrix(transform);

			// Normalize the matrix.
			if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
				return false;

			// First, isolate perspective.  This is the messiest.
			if (
				epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
				epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
				epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
			{
				// Clear the perspective partition
				LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
				LocalMatrix[3][3] = static_cast<T>(1);
			}

			// Next take care of translation (easy).
			translation = vec3(LocalMatrix[3]);
			LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

			vec3 Row[3], Pdum3;

			// Now get scale and shear.
			for (length_t i = 0; i < 3; ++i)
				for (length_t j = 0; j < 3; ++j)
					Row[i][j] = LocalMatrix[i][j];

			// Compute X scale factor and normalize first row.
			scale.x = length(Row[0]);
			Row[0] = detail::scale(Row[0], static_cast<T>(1));
			scale.y = length(Row[1]);
			Row[1] = detail::scale(Row[1], static_cast<T>(1));
			scale.z = length(Row[2]);
			Row[2] = detail::scale(Row[2], static_cast<T>(1));

			// At this point, the matrix (in rows[]) is orthonormal.
			// Check for a coordinate system flip.  If the determinant
			// is -1, then negate the matrix and the scaling factors.
			#if 0
			Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
			if (dot(Row[0], Pdum3) < 0)
			{
				for (length_t i = 0; i < 3; i++)
				{
					scale[i] *= static_cast<T>(-1);
					Row[i] *= static_cast<T>(-1);
				}
			}
	#endif

			rotation.y = asin(-Row[0][2]);
			if (cos(rotation.y) != 0) {
				rotation.x = atan2(Row[1][2], Row[2][2]);
				rotation.z = atan2(Row[0][1], Row[0][0]);
			}
			else {
				rotation.x = atan2(-Row[2][0], Row[1][1]);
				rotation.z = 0;
			}


			return true;
		}
}