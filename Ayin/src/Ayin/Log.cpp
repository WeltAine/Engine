#include "AyinPch.h"

#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Ayin {
	
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	

	void Log::Init() {

		spdlog::set_pattern("&^[%T] %n: %v%$");//设置日志格式

		s_CoreLogger = spdlog::stdout_color_mt("Ayin");
		s_CoreLogger->set_level(spdlog::level::trace);//设置输出的最低等级

		s_ClientLogger = spdlog::stderr_color_mt("App");
		s_ClientLogger->set_level(spdlog::level::trace);//设置输出的最低等级

	}

}