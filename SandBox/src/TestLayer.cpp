#include <AyinPch.h>

#include "TestLayer.h"

#include <Ayin/Core/Application.h>

#include <cstdlib>
#include <imgui.h>

namespace {
	constexpr auto kAutoExitEnv = "AYIN_SANDBOX_SMOKE_AUTO_EXIT";	//! 环境变量名称
	constexpr std::array<const char*, 4> kTraceLabels{ "PreUpdate", "Update", "PostUpdate", "Presentation" };

	// 只要环境变量不是空字符串也不是 '0'，就视为启用。
	bool IsAutoExitEnabled(const char* value) { return value != nullptr && value[0] != '\0' && value[0] != '0'; }
}

TestLayer::SmokeState* TestLayer::SmokeSystem::s_State = nullptr;

TestLayer::TestLayer()
	: Ayin::Layer("TestLayer") {
	const char* env = std::getenv(kAutoExitEnv);	//! std::getenv 从操作系统中获取环境变量，找的的话返回内容指针，没找到返回 nllptr
	m_AutoExitEnabled = IsAutoExitEnabled(env);
}

TestLayer::~TestLayer() = default;

void TestLayer::OnAttach() {
	// 先把测试 System 绑定到当前层的状态，再注册到所有需要观察的阶段。
	SmokeSystem::Bind(&m_State);
	m_Schedule.AddSystem<SmokeSystem>({ Ayin::SystemPhase::PreUpdate, Ayin::SystemPhase::Update, Ayin::SystemPhase::PostUpdate, Ayin::SystemPhase::Presentation }, { Ayin::SceneMode::Editor });
	m_TestSystemActive = true;
	RunOneShotChecks();
}

void TestLayer::OnDetach() {
	// 先移除 System，再清空状态指针，这样 OnDetach 统计时状态还是有效的。
	if (m_TestSystemActive) {
		m_Schedule.RemoveSystem<SmokeSystem>();
		m_TestSystemActive = false;
	}
	SmokeSystem::Unbind();
	if (m_State.DetachCount != 2) {
		SetFailure("detach count mismatch");
	}
}

void TestLayer::OnUpdate(Ayin::Timestep deltaTime) {
	// 只在首次附着后、第一帧结束前补跑一次一次性检查。
	if (!m_RanChecks) {
		RunOneShotChecks();
	}
	// 每一帧都跑一次真实调度，直到它通过为止。
	RunLiveFrame(deltaTime);
	// 真实帧验证通过后，在烟雾模式下请求退出。
	if (m_AutoExitEnabled && m_LiveFrameValidated && !m_AutoExitRequested) {
		m_AutoExitRequested = true;
		Ayin::Application::Get().Close();
	}
}

void TestLayer::RunOneShotChecks() {
	if (m_RanChecks) {
		return;
	}

	// 只有确认测试 System 确实注册成功，后面的统计才有意义。
	if (!m_TestSystemActive || !m_Schedule.Contain<SmokeSystem>()) {
		SetFailure("system not attached");
		return;
	}

	if (m_State.AttachCount != 1) {
		SetFailure("attach count mismatch");
		return;
	}

	m_State.RuntimeHits = 0;
	m_State.EditorHits = 0;
	m_State.LiveHits = 0;
	m_State.Trace.fill({});
	// Runtime 模式不应该派发这个只用于 Editor 的测试 System。
	Ayin::SystemContext runtimeContext{ .Scene = m_Scene, .DeltaTime = Ayin::Timestep{}, .Mode = Ayin::SceneMode::Runtime, .Phase = Ayin::SystemPhase::None };
	m_Schedule.Run(runtimeContext);
	if (m_State.RuntimeHits != 0) {
		SetFailure("runtime mode should not dispatch");
		return;
	}
	m_State.RuntimeRejected = true;

	m_State.Trace.fill({});
	m_State.EditorHits = 0;
	// Editor 模式应该按固定顺序命中四个阶段。
	Ayin::SystemContext editorContext{ .Scene = m_Scene, .DeltaTime = Ayin::Timestep{}, .Mode = Ayin::SceneMode::Editor, .Phase = Ayin::SystemPhase::None };
	m_Schedule.Run(editorContext);
	if (m_State.EditorHits != 4 || !m_Schedule.Contain<SmokeSystem>()) {
		SetFailure("editor run did not hit four phases");
		return;
	}

	m_State.EditorSequenced = m_State.Trace == ExpectedTrace();
	if (!m_State.EditorSequenced) {
		SetFailure("phase order mismatch");
		return;
	}

	m_Schedule.RemoveSystem<SmokeSystem>();
	m_TestSystemActive = false;
	// 卸载 System 时，必须同步触发一次 OnDetach。
	if (m_Schedule.Contain<SmokeSystem>()) {
		SetFailure("system still contained after remove");
		return;
	}
	if (m_State.DetachCount != 1) {
		SetFailure("detach count mismatch after remove");
		return;
	}
	m_State.Removed = true;

	m_Schedule.AddSystem<SmokeSystem>({ Ayin::SystemPhase::PreUpdate, Ayin::SystemPhase::Update, Ayin::SystemPhase::PostUpdate, Ayin::SystemPhase::Presentation }, { Ayin::SceneMode::Editor });
	m_TestSystemActive = true;
	// 重新添加后，Attach 计数和注册状态都应该恢复。
	if (m_State.AttachCount != 2 || !m_Schedule.Contain<SmokeSystem>()) {
		SetFailure("system did not reattach");
		return;
	}

	m_RanChecks = true;
	m_State.Completed = true;
}

void TestLayer::RunLiveFrame(Ayin::Timestep deltaTime) {
	if (!m_TestSystemActive || !m_Schedule.Contain<SmokeSystem>()) {
		return;
	}

	// 真实运行帧用来证明：应用循环启动后，调度器行为仍然一致。
	++m_State.LiveFrameCount;
	m_State.Trace.fill({});
	m_State.EditorHits = 0;
	Ayin::SystemContext editorContext{ .Scene = m_Scene, .DeltaTime = deltaTime, .Mode = Ayin::SceneMode::Editor, .Phase = Ayin::SystemPhase::None };
	m_Schedule.Run(editorContext);
	if (m_State.EditorHits == 4 && m_State.Trace == ExpectedTrace()) {
		m_LiveFrameValidated = true;
		m_State.Pass = true;
		m_State.Completed = true;
		// 即使关闭前又跑了一次 OnUpdate，也只记录一次日志。
		if (!m_LoggedResult) {
			AYIN_INFO("SYSTEM_SCHEDULE_SMOKE: PASS");
			m_LoggedResult = true;
		}
		return;
	}

	SetFailure("live frame did not produce expected four-phase trace");
}

void TestLayer::SetFailure(const char* failure) {
	if (m_State.Failure.empty()) {
		m_State.Failure = failure;
	}
	m_State.Pass = false;
	m_State.Completed = true;
	// 只输出第一次失败，避免日志被重复失败刷屏。
	if (!m_LoggedResult) {
		AYIN_ERROR("SYSTEM_SCHEDULE_SMOKE: FAIL - {0}", m_State.Failure);
		m_LoggedResult = true;
	}
}

void TestLayer::RenderLine(const char* label, bool passed) const {
	ImGui::Text("%s: %s", label, passed ? "pass" : "fail");
}

const char* TestLayer::ResultText() const {
	return m_State.Pass ? "PASS" : (m_State.Completed ? "FAIL" : "PENDING");
}

std::array<std::string, 4> TestLayer::ExpectedTrace() const {
	return { std::string(kTraceLabels[0]), std::string(kTraceLabels[1]), std::string(kTraceLabels[2]), std::string(kTraceLabels[3]) };
}

void TestLayer::OnImGuiRender() {
	// 这个面板只用于运行时快速查看测试结果。
	ImGui::Begin("Scheduler Smoke Test");
	ImGui::Text("Result: %s", ResultText());
	ImGui::Text("Frames: %u", m_State.LiveFrameCount);
	ImGui::Text("Auto exit: %s", m_AutoExitEnabled ? "enabled" : "disabled");
	ImGui::Text("Exit requested: %s", m_AutoExitRequested ? "yes" : "no");
	RenderLine("Attach", m_State.AttachCount >= 1);
	RenderLine("Detach", m_State.DetachCount >= 1);
	RenderLine("Runtime rejected", m_State.RuntimeRejected);
	RenderLine("Editor sequenced", m_State.EditorSequenced);
	RenderLine("Removed", m_State.Removed);
	RenderLine("Completed", m_State.Completed);
	if (!m_State.Failure.empty()) {
		ImGui::Text("Failure: %s", m_State.Failure.c_str());
	}
	ImGui::Separator();
	ImGui::Text("Trace:");
	for (const auto& entry : m_State.Trace) {
		if (!entry.empty()) {
			ImGui::BulletText("%s", entry.c_str());
		}
	}
	ImGui::End();
}

// --------------------------------------------------------------------------------------

void TestLayer::SmokeSystem::Bind(SmokeState* state) {
	s_State = state;
}

void TestLayer::SmokeSystem::Unbind() {
	s_State = nullptr;
}

void TestLayer::SmokeSystem::OnAttach() {
	if (s_State != nullptr) {
		++s_State->AttachCount;
	}
}

void TestLayer::SmokeSystem::OnDetach() {
	if (s_State != nullptr) {
		++s_State->DetachCount;
	}
}


void TestLayer::SmokeSystem::RecordPhase(const Ayin::SystemContext& context, const char* label) {
	if (s_State == nullptr) {
		return;
	}
	if (context.Mode == Ayin::SceneMode::Runtime) {
		++s_State->RuntimeHits;
		return;
	}
	if (context.Mode == Ayin::SceneMode::Editor) {
		++s_State->EditorHits;
	}
	const std::size_t index = static_cast<std::size_t>(s_State->EditorHits - 1);
	if (index < s_State->Trace.size()) {
		s_State->Trace[index] = label;
	}
}

void TestLayer::SmokeSystem::OnPreUpdate(const Ayin::SystemContext& context) {
	RecordPhase(context, "PreUpdate");
}

void TestLayer::SmokeSystem::OnUpdate(const Ayin::SystemContext& context) {
	RecordPhase(context, "Update");
}

void TestLayer::SmokeSystem::OnPostUpdate(const Ayin::SystemContext& context) {
	RecordPhase(context, "PostUpdate");
}

void TestLayer::SmokeSystem::OnPresentationUpdate(const Ayin::SystemContext& context) {
	RecordPhase(context, "Presentation");
}
