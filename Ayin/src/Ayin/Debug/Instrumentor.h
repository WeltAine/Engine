#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <algorithm>
#include <thread>


#include "Ayin/Core/Core.h"

// spdlog and fmt includes
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/bundled/format.h>
#include <spdlog/async.h> // Required for async features

namespace Ayin {

	/// <summary>
	/// 一个执行结果
	/// </summary>
	AYIN_API struct ProfileResult {

		std::string FuncName;
		double Time;
		//ToDo 这里加一个进程ID
		std::thread::id ThreadID;
		char Type;

	};

}

template<>
struct fmt::formatter<Ayin::ProfileResult> {

	// 解析格式规范（例如 {:x}）。我们将仅支持默认设置。
	constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

	template <typename FormatContext>
	auto format(const Ayin::ProfileResult& result, FormatContext& ctx) const {

		std::string name = result.FuncName;
		std::replace(name.begin(), name.end(), '"', '\'');

		return fmt::format_to(ctx.out(),
			"\n\t\t{{\n"						//		{
			"\t\t\t\"cat\": \"function\",\n"	//			"cat": "function"
			"\t\t\t\"name\": \"{}\",\n"			//			"name": "EntryPoint"
			"\t\t\t\"ph\": \"{}\",\n"			//			"ph": "B or E"
			"\t\t\t\"pid\": 0,\n"				//			"pid": 0
			"\t\t\t\"tid\": {},\n"				//			"tid": n,
			"\t\t\t\"ts\": {:.3f}\n"			//			"ts": n
			"\t\t}}",							//		}
			name,
			result.Type,
			result.ThreadID,
			result.Time
		);
	}



};


namespace Ayin {

	AYIN_API struct InstrumentationSession {

		std::string SessionName;

	};

	/// <summary>
	/// 插桩输出控制
	/// </summary>
	AYIN_API class Instrumentor {

	private:
		Instrumentor()
			: m_CurrentSession{ nullptr }, m_ProfileCount{ 0 } 
		{}


	public:

		inline void BeginSession(const std::string& name, const std::string& filepath = "results.json") {
		
			// 如果线程池尚未初始化，则进行初始化
			// 生产者消费者模型，logger->info(...)是生产者
			// 队列中 8192 个插槽（生产者产生的数据）通常足以满足性能分析的需求
			// 1 是后台线程的数量。（将数据写入文件的消费者）
			spdlog::init_thread_pool(8192, 1);


			// 我们使用一个基础的文件接收器。
			auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filepath, true);//设置输出目标，是否清空文件

			// 使用 create_async 以确保日志记录器不会阻塞游戏主线程
			m_Logger = std::make_shared<spdlog::async_logger>(
				"Instrumentor",							// logger 名称
				sink,									// 已创建的 sink 对象
				spdlog::thread_pool(),					// 使用共享全局线程池
				spdlog::async_overflow_policy::block	// 溢出策略（超过sink中的槽数则阻塞，并输出）
			);


			// set_pattern("%v") 确保只记录消息内容，
			// 不包含 spdlog 默认的时间戳或标签，否则会破坏 JSON 格式。
			m_Logger->set_pattern("%v");
			m_Logger->flush_on(spdlog::level::err);		// 如果发生错误，立即刷新

			WriteHeader();
			m_CurrentSession = new InstrumentationSession{ name };
		}


		inline void EndSession() {

			// 至关重要：在异步模式下，我们必须在关闭前通知日志记录器将队列中所有剩余的消息刷新到磁盘。
			if (m_Logger) {
				m_Logger->flush();
			}

			WriteFooter();
			delete m_CurrentSession;
			m_CurrentSession = nullptr;
			m_ProfileCount = 0;
			m_Logger.reset();

			// 可选：从 spdlog 的内部注册表中移除该日志记录器
			spdlog::drop("Instrumentor");
		}

		inline void WriteHeader() {

			m_Logger->info(R"({
    "otherData": {},
    "traceEvents": [
)");

		}

		inline void WriteProfile(const ProfileResult& result) {

			if (m_ProfileCount++ > 0)
				m_Logger->info(",");

			m_Logger->info("{}", result);

		}

		inline void WriteFooter() {

			m_Logger->info(R"(
    ]
}
)");

		}


		inline static Instrumentor& Get() {

			static Instrumentor instance;
			return instance;

		}


	private:

		InstrumentationSession* m_CurrentSession;

		std::shared_ptr<spdlog::logger> m_Logger;

		int m_ProfileCount;

	};



	/// <summary>
	/// 插桩计时器
	/// </summary>
	AYIN_API class InstrumentationTimer {

	public:
		InstrumentationTimer(const char* name)
			:m_Name{ name }
		{
			// 构造时立即记录 Begin 事件
			double start = std::chrono::duration<double, std::micro>(
				std::chrono::high_resolution_clock::now().time_since_epoch()
			).count();

			std::thread::id tid = std::this_thread::get_id();
			Instrumentor::Get().WriteProfile({ m_Name, start, tid, 'B' });
		}

		~InstrumentationTimer() {

			if(!m_Stopped)
				Stop();

		}

		inline void Stop() {

			// 析构时记录 End 事件
			double end = std::chrono::duration<double, std::micro>(
				std::chrono::high_resolution_clock::now().time_since_epoch()
			).count();

			std::thread::id tid = std::this_thread::get_id();
			Instrumentor::Get().WriteProfile({ m_Name, end, tid, 'E' });

			m_Stopped = true;
		}

	private:

		const char* m_Name;
		bool m_Stopped = false;
	};
}


#define AYIN_PROFILE 1
#if AYIN_PROFILE
#define AYIN_PROFILE_BEGIN_SESSION(name, filepath) ::Ayin::Instrumentor::Get().BeginSession(name, filepath)
#define AYIN_PROFILE_END_SESSION() ::Ayin::Instrumentor::Get().EndSession()
#define AYIN_PROFILE_SCOPE(name) ::Ayin::InstrumentationTimer timer##__LINE__(name);
#define AYIN_PROFILE_FUNCTION() AYIN_PROFILE_SCOPE(__FUNCSIG__)
#else
#define AYIN_PROFILE_BEGIN_SESSION(name, filepath)
#define AYIN_PROFILE_END_SESSION()
#define AYIN_PROFILE_SCOPE(name)
#define AYIN_PROFILE_FUNCTION()
#endif