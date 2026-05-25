#pragma once

// TODO
#include "PlatformDetection.hpp"

#include <memory>
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include "Rect.h"
#include "FunctionArray.h"

//struct IVec2Hash
//{
//    std::size_t operator()(const glm::ivec2& v) const noexcept
//    {
//        std::size_t h1 = std::hash<int>()(v.x);
//        std::size_t h2 = std::hash<int>()(v.y);
//        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
//
//    }
//};

namespace std {
    template<>
    struct hash<glm::ivec2> {
        size_t operator()(const glm::ivec2& v) const noexcept {
            // Simple, fast, and well‑distributed hash for two ints
            size_t h1 = hash<int>()(v.x);
            size_t h2 = hash<int>()(v.y);
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
}

#include <glm/glm.hpp>
#include <functional>

namespace std {
    template<>
    struct less<glm::ivec2> {
        bool operator()(const glm::ivec2& lhs, const glm::ivec2& rhs) const {
            // Lexicographic ordering: first by x, then by y
            return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y);
        }
    };
}

#define BIT(x) (1 << x)

#define BP_EXPAND_MACRO(x) x
#define BP_STRINGIFY_MACRO(x) #x

#define BP_CONCAT_IMPL(x, y) x##y
#define BP_CONCAT(x, y) BP_CONCAT_IMPL(x, y)

#include "Logger.h"
#include "Assert.h"
#include "Version.h"
#include "Bounds.h"

#define BP_BIND_EVENT_FN(fn) [this](auto&& e) { return this->fn(std::forward<decltype(e)>(e)); }
#define BP_BIND_VOID_NO_ARGS_FN(fn) [this]() { this->fn(); }
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