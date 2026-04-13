#pragma once
#include <filesystem>
#include <string>
#include "Logger.h"

#define BP_ENABLE_ASSERTS
#define BP_ENABLE_VERIFY

#ifdef BP_ENABLE_ASSERTS

void DebugBreak();

#define BP_INTERNAL_ASSERT_IMPL(type, check, ...)          \
	do {                                                   \
		if (!(check)) {                                    \
			BP##type##ERROR(__VA_ARGS__);                 \
			DebugBreak();                                  \
		}                                                  \
	} while (0)

#define BP_INTERNAL_ASSERT_HAS_ARGS(...) BP_INTERNAL_ASSERT_HAS_ARGS_IMPL(__VA_ARGS__, 1, 0)
#define BP_INTERNAL_ASSERT_HAS_ARGS_IMPL(_1, _2, N, ...) N

#define BP_INTERNAL_ASSERT_SELECT_MACRO(check, ...) \
	BP_INTERNAL_ASSERT_SELECT_MACRO_IMPL(check, BP_INTERNAL_ASSERT_HAS_ARGS(__VA_ARGS__), __VA_ARGS__)

#define BP_INTERNAL_ASSERT_SELECT_MACRO_IMPL(check, hasMsg, ...) \
	BP_INTERNAL_ASSERT_SELECT_MACRO_IMPL2(check, hasMsg, __VA_ARGS__)

#define BP_INTERNAL_ASSERT_SELECT_MACRO_IMPL2(check, hasMsg, ...) \
	BP_INTERNAL_ASSERT_IMPL_SELECT_##hasMsg(check, __VA_ARGS__)

#define BP_INTERNAL_ASSERT_IMPL_SELECT_0(check, ...) BP_INTERNAL_ASSERT_IMPL(_, check, "Assertion failed")
#define BP_INTERNAL_ASSERT_IMPL_SELECT_1(check, ...) BP_INTERNAL_ASSERT_IMPL(_, check, __VA_ARGS__)

#define BP_ASSERT(...) BP_INTERNAL_ASSERT_SELECT_MACRO(__VA_ARGS__)
#define BP_CORE_ASSERT(...) BP_INTERNAL_ASSERT_SELECT_MACRO(__VA_ARGS__)

#else
#define BP_ASSERT(...)
#define BP_CORE_ASSERT(...)
#endif // BP_ENABLE_ASSERTS

#ifdef BP_ENABLE_VERIFY

#define BP_INTERNAL_VERIFY_IMPL(type, check, ...)          \
	do {                                                   \
		if (!(check)) {                                    \
			BP##type##ERROR(__VA_ARGS__);                 \
			DebugBreak();                                  \
		}                                                  \
	} while (0)

#define BP_INTERNAL_VERIFY_HAS_ARGS(...) BP_INTERNAL_ASSERT_HAS_ARGS(__VA_ARGS__)
#define BP_INTERNAL_VERIFY_SELECT_MACRO(check, ...) \
	BP_INTERNAL_VERIFY_SELECT_MACRO_IMPL(check, BP_INTERNAL_VERIFY_HAS_ARGS(__VA_ARGS__), __VA_ARGS__)

#define BP_INTERNAL_VERIFY_SELECT_MACRO_IMPL(check, hasMsg, ...) \
	BP_INTERNAL_VERIFY_SELECT_MACRO_IMPL2(check, hasMsg, __VA_ARGS__)

#define BP_INTERNAL_VERIFY_SELECT_MACRO_IMPL2(check, hasMsg, ...) \
	BP_INTERNAL_VERIFY_IMPL_SELECT_##hasMsg(check, __VA_ARGS__)

#define BP_INTERNAL_VERIFY_IMPL_SELECT_0(check, ...) BP_INTERNAL_VERIFY_IMPL(_, check, "Verification failed")
#define BP_INTERNAL_VERIFY_IMPL_SELECT_1(check, ...) BP_INTERNAL_VERIFY_IMPL(_, check, __VA_ARGS__)

#define BP_VERIFY(...) BP_INTERNAL_VERIFY_SELECT_MACRO(__VA_ARGS__)
#define BP_CORE_VERIFY(...) BP_INTERNAL_VERIFY_SELECT_MACRO(__VA_ARGS__)

#else
#define BP_VERIFY(...)
#define BP_CORE_VERIFY(...)
#endif // BP_ENABLE_VERIFY