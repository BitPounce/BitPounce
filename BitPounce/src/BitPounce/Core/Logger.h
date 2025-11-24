#pragma once

#include <spdlog/spdlog.h>
#include "Base.h"
#include "spdlog/fmt/ostr.h"

namespace BitPounce
{
	class Logger
	{
	public:
		static void Initialize();

		inline static Ref<spdlog::logger>& GetCoreLogger() { return  s_CoreLogger; }
		inline static Ref<spdlog::logger>& GetClientLogger() { return  s_ClientLogger; }

	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
	};
}

#define BP_FILENAME (std::string(__FILE__).substr(std::string(__FILE__).find_last_of("/\\") + 1))

#ifdef BP_PLATFORM_WEB
// Core log macros (all include filename and line number)
#define BP_CORE_TRACE(fmt, ...)    ::BitPounce::Logger::GetCoreLogger()->trace(fmt " [{}:{}]", ##__VA_ARGS__, BP_FILENAME, __LINE__)
#define BP_CORE_INFO(fmt, ...)     ::BitPounce::Logger::GetCoreLogger()->info(fmt " [{}:{}]", ##__VA_ARGS__, BP_FILENAME, __LINE__)
#define BP_CORE_WARN(fmt, ...)     ::BitPounce::Logger::GetCoreLogger()->warn(fmt " [{}:{}]", ##__VA_ARGS__, BP_FILENAME, __LINE__)
#define BP_CORE_ERROR(fmt, ...)    ::BitPounce::Logger::GetCoreLogger()->error(fmt " [{}:{}]", ##__VA_ARGS__, BP_FILENAME, __LINE__)
#define BP_CORE_CRITICAL(fmt, ...) ::BitPounce::Logger::GetCoreLogger()->critical(fmt " [{}:{}]", ##__VA_ARGS__, BP_FILENAME, __LINE__)

// Client log macros (all include filename and line number)
#define BP_TRACE(fmt, ...)         ::BitPounce::Logger::GetClientLogger()->trace(fmt " [{}:{}]", ##__VA_ARGS__, BP_FILENAME, __LINE__)
#define BP_INFO(fmt, ...)          ::BitPounce::Logger::GetClientLogger()->info(fmt " [{}:{}]", ##__VA_ARGS__, BP_FILENAME, __LINE__)
#define BP_WARN(fmt, ...)          ::BitPounce::Logger::GetClientLogger()->warn(fmt " [{}:{}]", ##__VA_ARGS__, BP_FILENAME, __LINE__)
#define BP_ERROR(fmt, ...)         ::BitPounce::Logger::GetClientLogger()->error(fmt " [{}:{}]", ##__VA_ARGS__, BP_FILENAME, __LINE__)
#define BP_CRITICAL(fmt, ...)      ::BitPounce::Logger::GetClientLogger()->critical(fmt " [{}:{}]", ##__VA_ARGS__, BP_FILENAME, __LINE__)

#else
// Core log macros
#define BP_CORE_TRACE(...)    { ::BitPounce::Logger::GetCoreLogger()->trace(__VA_ARGS__); }
#define BP_CORE_INFO(...)     { ::BitPounce::Logger::GetCoreLogger()->info(__VA_ARGS__); }
#define BP_CORE_WARN(...)     { ::BitPounce::Logger::GetCoreLogger()->warn(__VA_ARGS__); }
#define BP_CORE_ERROR(...)    { ::BitPounce::Logger::GetCoreLogger()->error(__VA_ARGS__); }
#define BP_CORE_CRITICAL(...) { ::BitPounce::Logger::GetCoreLogger()->critical(__VA_ARGS__); }

// Client log macros
#define BP_TRACE(...)         { ::BitPounce::Logger::GetClientLogger()->trace(__VA_ARGS__); }
#define BP_INFO(...)          { ::BitPounce::Logger::GetClientLogger()->info(__VA_ARGS__); }
#define BP_WARN(...)          { ::BitPounce::Logger::GetClientLogger()->warn(__VA_ARGS__); }
#define BP_ERROR(...)         { ::BitPounce::Logger::GetClientLogger()->error(__VA_ARGS__); }
#define BP_CRITICAL(...)      { ::BitPounce::Logger::GetClientLogger()->critical(__VA_ARGS__); }

#endif // 


