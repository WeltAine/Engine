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
		uint64_t ThreadID;
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

	/// <summary>
	/// 插桩输出控制
	/// </summary>
	AYIN_API class InstrumentationSession {

	public:

		enum class Mode {

			Inherit, NewRoot

		};


		InstrumentationSession(const std::string& sessionName, const std::string& filePath = "results.json", Mode mode = Mode::Inherit)
			:m_SessionName{ sessionName }, m_Mode{mode}
		{

			// spdlog异步线程池初始化（保证整个程序只初始化一次）
			static bool s_ThreadPoolInitialized = []() {
					// 如果线程池尚未初始化，则进行初始化
					// 生产者消费者模型，logger->info(...)是生产者
					// 队列中 8192 个插槽（生产者产生的数据）通常足以满足性能分析的需求
					// 1 是后台线程的数量。（将数据写入文件的消费者）
					spdlog::init_thread_pool(8192, 1);
					return true;
				}();


			// 设置嵌套Session链
			m_ParentSession = s_CurrentSession;
			s_CurrentSession = this;


			// 输出目标设置(spdlog::sinks)
			auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath, true);	//文件路径，是否截断文件（新文件形式）


			// 使用 create_async 以确保日志记录器不会阻塞游戏主线程
			m_Logger = std::make_shared<spdlog::async_logger>(
				sessionName,							// logger 名称
				sink,									// 已创建的 sink 对象
				spdlog::thread_pool(),					// 使用共享全局线程池
				spdlog::async_overflow_policy::block	// 溢出策略（超过sink中的槽数则阻塞，并输出）
			);


			// set_pattern("%v") 确保只记录消息内容，
			// 不包含 spdlog 默认的时间戳或标签，否则会破坏 JSON 格式。
			m_Logger->set_pattern("%v");
			m_Logger->flush_on(spdlog::level::err);		// 如果发生错误，立即刷新


			WriteHeader();
		}

		~InstrumentationSession() {

			// 至关重要：在异步模式下，我们必须在关闭前通知日志记录器将队列中所有剩余的消息刷新到磁盘。
			m_Logger->flush();

			WriteFooter();

			if (!m_isEnd)
			{

				s_CurrentSession = m_ParentSession;
				m_isEnd = true;

			}

			spdlog::drop(m_Logger->name());
		}


		inline void EndSession() {
			
			if (!m_isEnd) {

				s_CurrentSession = m_ParentSession;
				m_isEnd = true;

			}
		};
		

		inline void WriteProfile(const ProfileResult& result) {

			m_Logger->info(",{}", result);

			// 对嵌套Session的递归输出
			if (m_ParentSession && m_Mode == Mode::Inherit) {

				m_ParentSession->WriteProfile(result);

			}

		}


	public:

		static InstrumentationSession* GetCurrentSession() { return s_CurrentSession; };

	private:

		inline void WriteHeader() {

			m_Logger->info(R"({
    "otherData": {},
    "traceEvents": [
)");
		};


		inline void WriteFooter() {

			m_Logger->info(R"(
    ]
}
)");

		};


	private:

		std::string m_SessionName;

		InstrumentationSession* m_ParentSession = nullptr;

		std::shared_ptr<spdlog::logger> m_Logger;

		Mode m_Mode = Mode::Inherit; 

		bool m_isEnd = false;

	private:

		static thread_local InstrumentationSession* s_CurrentSession;	//当前线程活跃对话

	};

	inline thread_local InstrumentationSession* InstrumentationSession::s_CurrentSession = nullptr;




	/// <summary>
	/// 插桩计时器（会将数据写到对应的Session中）
	/// </summary>
	AYIN_API class InstrumentationTimer {

	public:
		InstrumentationTimer(const char* name)
			:m_Name{ name }, m_Tid{ std::hash<std::thread::id>{}(std::this_thread::get_id()) }, m_Session{ InstrumentationSession::GetCurrentSession() }
		{
			// 构造时立即记录 Begin 事件
			double start = std::chrono::duration<double, std::micro>(
				std::chrono::high_resolution_clock::now().time_since_epoch()
			).count();


			if (m_Session) {

				m_Session->WriteProfile({ m_Name, start, m_Tid, 'B' });
				return;

			}

			AYIN_CORE_ERROR("Failed to export results"); // 当前的线程中未发现处于活动状态的 InstrumentationSession

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

			if (m_Session) {

				m_Session->WriteProfile({ m_Name, end, m_Tid, 'E' });

				m_Stopped = true;

				return;
			}

			AYIN_CORE_ERROR("Failed to export results"); // 当前的线程中未发现处于活动状态的 InstrumentationSession

		}

	private:

		const char* m_Name;
		uint64_t m_Tid;
		bool m_Stopped = false;

		InstrumentationSession* m_Session;
	};
}

#define AYIN_INTERNAL_CAT(a, b) a##b
#define AYIN_CAT(a,b) AYIN_INTERNAL_CAT(a, b)

#define AYIN_PROFILE 1
#if AYIN_PROFILE
#define AYIN_PROFILE_BEGIN_ROOT_SESSION(name, filepath)	::Ayin::InstrumentationSession AYIN_CAT(session_, __LINE__) (name, filepath, ::Ayin::InstrumentationSession::Mode::NewRoot)
#define AYIN_PROFILE_BEGIN_SESSION(name, filepath)		::Ayin::InstrumentationSession AYIN_CAT(session_, __LINE__) (name, filepath)
#define AYIN_PROFILE_END_SESSION()						::Ayin::InstrumentationSession::GetCurrentSession()->EndSession()
#define AYIN_PROFILE_SCOPE(name)						::Ayin::InstrumentationTimer AYIN_CAT(time_, __LINE__) (name);
#define AYIN_PROFILE_FUNCTION()							AYIN_PROFILE_SCOPE(__FUNCSIG__)
#else
#define AYIN_PROFILE_BEGIN_SESSION(name, filepath)
#define AYIN_PROFILE_END_SESSION()
#define AYIN_PROFILE_SCOPE(name)
#define AYIN_PROFILE_FUNCTION()
#endif