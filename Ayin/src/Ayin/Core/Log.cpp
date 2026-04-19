#include <AyinPch.h>

#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Ayin {
	
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	

	void Log::Init() {

		// spdlog异步线程池初始化（保证整个程序只初始化一次）
		static bool s_ThreadPoolInitialized = []() {
				// 如果线程池尚未初始化，则进行初始化
				// 生产者消费者模型，logger->info(...)是生产者
				// 队列中 8192 个插槽（生产者产生的数据）通常足以满足性能分析的需求
				// 1 是后台线程的数量。（将数据写入文件的消费者）
				spdlog::init_thread_pool(8192, 1);
				return true;
			}();



		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("Ayin.log", true);

		std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};

		s_CoreLogger = std::make_shared<spdlog::async_logger>(
			"Ayin",
			sinks.begin(),
			sinks.end(),
			spdlog::thread_pool(),
			spdlog::async_overflow_policy::block
		);
		s_CoreLogger->set_level(spdlog::level::trace);//设置输出的最低等级


		s_CoreLogger = std::make_shared<spdlog::async_logger>(
			"App",
			sinks.begin(),
			sinks.end(),
			spdlog::thread_pool(),
			spdlog::async_overflow_policy::block
		);
		s_ClientLogger->set_level(spdlog::level::trace);//设置输出的最低等级


		// 注册到全局（可选），并设置统一格式
		spdlog::register_logger(s_CoreLogger);
		spdlog::register_logger(s_ClientLogger);
		spdlog::set_pattern("&^[%T] %n: %v%$");//设置日志格式

	}

}