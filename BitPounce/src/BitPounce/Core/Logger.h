#pragma once

#include <spdlog/spdlog.h>
#include "Base.h"

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


#define BP_CORE_TRACE(...)    { ::BitPounce::Logger::GetCoreLogger()->trace(__VA_ARGS__);	 }
#define BP_CORE_INFO(...)     { ::BitPounce::Logger::GetCoreLogger()->info(__VA_ARGS__);	 }
#define BP_CORE_WARN(...)     { ::BitPounce::Logger::GetCoreLogger()->warn(__VA_ARGS__);	 }
#define BP_CORE_ERROR(...)    { ::BitPounce::Logger::GetCoreLogger()->error(__VA_ARGS__);    }
#define BP_CORE_CRITICAL(...) { ::BitPounce::Logger::GetCoreLogger()->critical(__VA_ARGS__); }


// Client log macros
#define BP_TRACE(...)         { ::BitPounce::Logger::GetClientLogger()->trace(__VA_ARGS__);	   }
#define BP_INFO(...)          { ::BitPounce::Logger::GetClientLogger()->info(__VA_ARGS__);	   }
#define BP_WARN(...)          { ::BitPounce::Logger::GetClientLogger()->warn(__VA_ARGS__);	   }
#define BP_ERROR(...)         { ::BitPounce::Logger::GetClientLogger()->error(__VA_ARGS__);	   }
#define BP_CRITICAL(...)      { ::BitPounce::Logger::GetClientLogger()->critical(__VA_ARGS__); }