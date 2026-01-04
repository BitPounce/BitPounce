#define BP_ENABLE_ASSERTS

#ifdef BP_ENABLE_ASSERTS

void DebugBreak();

#define BP_ASSERT(x, ...) { if(!(x)) { BP_ERROR("Assertion Failed: {0}", __VA_ARGS__); DebugBreak(); } }
#define BP_CORE_ASSERT(x, ...) { if(!(x)) { BP_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); DebugBreak(); } }
#endif