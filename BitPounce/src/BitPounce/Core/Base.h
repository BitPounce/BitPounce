#pragma once

// TODO
#include "PlatformDetection.hpp"

#include <memory>

#define BIT(x) (1 << x)

#define BP_EXPAND_MACRO(x) x
#define BP_STRINGIFY_MACRO(x) #x

#define BP_ENABLE_ASSERTS

#ifdef BP_ENABLE_ASSERTS

#define BP_ASSERT(x, ...) { if(!(x)) { BP_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#define BP_CORE_ASSERT(x, ...) { if(!(x)) { BP_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#endif

namespace BitPounce
{
	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

}