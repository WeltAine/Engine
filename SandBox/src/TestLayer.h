#pragma once

#include "Ayin/Core/Layer.h"
#include "Ayin/Core/Timestep.h"
#include "Ayin/Scene/Scene.h"
#include "Ayin/System/SystemSchedule.h"

#include <array>
#include <string>

//! “烟雾测试”（Smoke Test）
//! 名字来自硬件测试：设备通电后，如果直接冒烟，说明存在严重问题，没必要继续做更深入测试。软件里的意思类似。
//! 先确认程序能启动、核心功能没有立即崩溃，再进行更详细的测试。



// 主要两个测试
// 测试1 RunOneShotChecks() 进行一次完整的从 配置->插入->阶段更新->退出 的流程，检查，配置有效性，更新顺序正确性
// 测试2 在测试一完成后在模拟的真实环境中进行循环更新

class TestLayer final : public Ayin::Layer {
public:

	TestLayer();
	~TestLayer() override;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Ayin::Timestep deltaTime) override;
	void OnImGuiRender() override;

private:
	// 烟雾测试观察到的状态快照，同时用于日志输出和 ImGui 面板显示。
	struct SmokeState final {
		int AttachCount = 0;				// System 被 Attach 次数计数
		int DetachCount = 0;				// 同上
		int RuntimeHits = 0;				// 同下
		int EditorHits = 0;					// Editor 模式计数：System 是在什么模式下被使用
		int LiveHits = 0;
		int LiveFrameCount = 0;
		bool RuntimeRejected = false;
		bool EditorSequenced = false;
		bool Removed = false;				// 移除功能是否正常
		bool Completed = false;				// 测试可以停止了，要嘛出问题了，要么成功了，可以配合 Pass 观察
		bool Pass = false;
		std::string Failure;
		std::array<std::string, 4> Trace{};	// 阶段更新顺序追踪
	};

	// 仅用于测试的小型 System：记录自己被哪个调度阶段调用。
	// 用它验证阶段顺序、挂载/卸载，以及 SceneMode 过滤是否正常。
	struct SmokeSystem final : Ayin::ISystem {
		static void Bind(SmokeState* state);
		static void Unbind();

		void OnAttach() override;
		void OnDetach() override;
		void OnPreUpdate(const Ayin::SystemContext& context) override;
		void OnUpdate(const Ayin::SystemContext& context) override;
		void OnPostUpdate(const Ayin::SystemContext& context) override;
		void OnPresentationUpdate(const Ayin::SystemContext& context) override;

	private:

		// 每次 System 的阶段更新被使用时进行记录，记录什么模式下使用以及什么执行的是什么更新阶段
		static void RecordPhase(const Ayin::SystemContext& context, const char* label);
		static SmokeState* s_State;
	};

	// 第一轮检查：验证不依赖窗口帧循环或用户输入的调度器行为。
	void RunOneShotChecks();
	// 第二轮检查：在应用真实运行帧中再次验证调度行为。
	void RunLiveFrame(Ayin::Timestep deltaTime);
	// 记录第一次失败，并保持失败信息稳定，避免 UI 和日志来回变化。
	void SetFailure(const char* failure);
	// 在 ImGui 面板中渲染一条检查结果。
	void RenderLine(const char* label, bool passed) const;
	// 把当前测试状态转换为一个简短结果字符串。
	const char* ResultText() const;
	// 构造预期的阶段轨迹，用于比对，检查阶段执行过程是否符合预期。
	std::array<std::string, 4> ExpectedTrace() const;

private:
	Ayin::Scene m_Scene;
	Ayin::SystemSchedule m_Schedule;
	SmokeState m_State;
	// 启用后，真实帧验证通过时 应用会自动关闭。
	bool m_AutoExitEnabled = false;				// 是否自动关闭程序（在运行完必要测试后）
	bool m_AutoExitRequested = false;			// 是否关闭过
	// 标记测试 System 当前是否注册在调度器里。
	bool m_TestSystemActive = false;
	// 防止重复输出 PASS/FAIL 日志。
	bool m_LoggedResult = false;
	// 防止一次性检查（也就是第一道测试）重复运行。
	bool m_RanChecks = false;
	// 真实运行帧产出期望轨迹后置为 true。
	bool m_LiveFrameValidated = false;
};
