#pragma once

// TODO
#include "PlatformDetection.hpp"

#include <memory>

#define BIT(x) (1 << x)

#define BP_EXPAND_MACRO(x) x
#define BP_STRINGIFY_MACRO(x) #x

#define BP_CONCAT_IMPL(x, y) x##y
#define BP_CONCAT(x, y) BP_CONCAT_IMPL(x, y)

#include "Logger.h"
#include "Assert.h"

#define BP_BIND_EVENT_FN(fn) [this](auto&& e) { return this->fn(std::forward<decltype(e)>(e)); }
#define NULL0 0


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