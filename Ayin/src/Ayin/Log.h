#pragma once

#include "Core.h"
#include "memory"
#include "spdlog/spdlog.h"

namespace Ayin {

	class AYIN_API Log
	{
	public:
		static void Init();

		//为何这两个要用inline？？？我记得inline的一个作用就是去重
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; };
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; };

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

#define AYIN_CORE_TRACE(...)	Ayin::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define AYIN_CORE_INFO(...)		Ayin::Log::GetCoreLogger()->info(__VA_ARGS__)
#define AYIN_CORE_WARN(...)		Ayin::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define AYIN_CORE_ERROR(...)	Ayin::Log::GetCoreLogger()->error(__VA_ARGS__)
#define AYIN_CORE_FATAL(...)	Ayin::Log::GetCoreLogger()->fatal(__VA_ARGS__)


#define AYIN_TRACE(...)			Ayin::Log::GetClientLogger()->trace(__VA_ARGS__)
#define AYIN_INFO(...)			Ayin::Log::GetClientLogger()->info(__VA_ARGS__)
#define AYIN_WARN(...)			Ayin::Log::GetClientLogger()->warn(__VA_ARGS__)
#define AYIN_ERROR(...)			Ayin::Log::GetClientLogger()->error(__VA_ARGS__)
#define AYIN_FATAL(...)			Ayin::Log::GetClientLogger()->fatal(__VA_ARGS__)


