#include <AyinPch.h>

#include "SystemTestLayer.h"
#include "TestEnvironment.h"

#include <Ayin/Core/Application.h>

#include <cmath>
#include <imgui.h>

namespace {
	// SystemSchedule 固定按这四个阶段执行；测试轨迹也使用相同顺序。
	constexpr std::array<const char*, 4> kPhases{ "PreUpdate", "Update", "PostUpdate", "Presentation" };


	bool IsPhase(const Ayin::SystemPhase phase, const char* expected) {
		// SystemContext 会携带当前阶段，这里把枚举转换成测试容易比较的字符串。
		const std::array phases{
			Ayin::SystemPhase::PreUpdate,
			Ayin::SystemPhase::Update,
			Ayin::SystemPhase::PostUpdate,
			Ayin::SystemPhase::Presentation,
		};

		for (std::size_t index = 0; index < phases.size(); ++index) {
			if (phase == phases[index] && std::string{ expected } == kPhases[index]) {
				return true;
			}
		}
		return false;
	}

	const char* ModeName(const Ayin::SceneMode mode) {
		switch (mode) {
		case Ayin::SceneMode::Editor: return "Editor";
		case Ayin::SceneMode::Simulation: return "Simulation";
		case Ayin::SceneMode::Runtime: return "Runtime";
		default: return "None";
		}
	}
}

SystemTestLayer::TestState* SystemTestLayer::ProbeSystem::s_State = nullptr;

SystemTestLayer::SystemTestLayer()
	: Ayin::Layer("SystemTestLayer") {
	// 只读取当前进程环境；自动化运行时由通用环境变量控制退出。
	m_AutoExitEnabled = SandBoxTests::IsTruthyEnvironmentVariable(SandBoxTests::AutoExitVariable);
}

SystemTestLayer::~SystemTestLayer() = default;

void SystemTestLayer::OnAttach() {
	// 先绑定观测状态，再创建 World，确保 Pipeline 中的 System 在 OnAttach
	// 时就能把生命周期数据写入当前 SystemTestLayer。
	ProbeSystem::Bind(&m_State);
	BuildWorld();
	RunOneShotChecks();
}

void SystemTestLayer::OnDetach() {
	// World 当前只负责销毁 Schedule，测试用例中的显式移除已经验证了 OnDetach。
	m_World.reset();
	ProbeSystem::Unbind();
}

void SystemTestLayer::OnUpdate(const Ayin::Timestep deltaTime) {
	// 一次性检查通常已经在 OnAttach 完成；这里保留补偿调用，避免初始化顺序变化时漏测。
	if (!m_RanChecks) {
		RunOneShotChecks();
	}

	if (m_RanChecks && m_State.Failure.empty() && !m_State.LiveFramePassed) {
		// 通过真实应用帧调用 World，而不是直接调用 Schedule，验证最终使用路径。
		RunLiveFrame(deltaTime);
	}

	if (m_AutoExitEnabled && m_State.Completed && !m_AutoExitRequested) {
		// 自动退出只用于命令行烟雾测试；普通运行时不设置环境变量即可保持窗口打开。
		m_AutoExitRequested = true;
		Ayin::Application::Get().Close();
	}
}

void SystemTestLayer::BuildWorld() {
	Ayin::SystemPipeline::Builder builder;
	// EarlySystem 和 LateSystem 使用默认顺序，验证注册顺序会影响同一阶段内的执行顺序。
	// 两者都允许 Editor/Simulation，RuntimeSystem 只允许 Runtime，用来验证模式掩码。
	builder
		.AddSystem<EarlySystem>(
			{ Ayin::SystemPhase::PreUpdate, Ayin::SystemPhase::Update, Ayin::SystemPhase::PostUpdate, Ayin::SystemPhase::Presentation },
			{ Ayin::SceneMode::Editor, Ayin::SceneMode::Simulation })
		.AddSystem<LateSystem>(
			{ Ayin::SystemPhase::PreUpdate, Ayin::SystemPhase::Update, Ayin::SystemPhase::PostUpdate, Ayin::SystemPhase::Presentation },
			{ Ayin::SceneMode::Editor, Ayin::SceneMode::Simulation })
		.AddSystem<RuntimeSystem>(
			{ Ayin::SystemPhase::PreUpdate, Ayin::SystemPhase::Update, Ayin::SystemPhase::PostUpdate, Ayin::SystemPhase::Presentation },
			{ Ayin::SceneMode::Runtime });

	m_Pipeline = builder.Build();
	m_State.PipelineBuilt = true;
	m_Scene = Ayin::CreateRef<Ayin::Scene>();
	// World 会从 Pipeline 创建自己的 SystemSchedule，之后测试只通过 World 驱动它。
	m_World = std::make_unique<Ayin::World>(m_Scene, m_Pipeline);

	const std::vector<std::string> expectedAttachOrder{
		"Early:Attach", "Late:Attach", "Runtime:Attach"
	};
	m_State.AttachOrderPassed = m_State.LifecycleTrace == expectedAttachOrder;
	if (!m_State.AttachOrderPassed) {
		SetFailure("pipeline systems did not attach in registration order");
	}
}

void SystemTestLayer::RunOneShotChecks() {
	if (m_RanChecks || !m_State.PipelineBuilt || m_World == nullptr) {
		return;
	}

	const bool pipelinePassed = CheckPipelineAndWorld();
	const bool schedulePassed = CheckScheduleLifecycle();
	const bool cleanupPassed = CheckDestructorCleanupAndMove();
	m_RanChecks = true;

	// 一次性检查失败时也标记完成，使自动化运行能够退出并报告 FAIL，而不是一直挂起窗口。
	if (!pipelinePassed || !schedulePassed || !cleanupPassed || !m_State.Failure.empty()) {
		m_State.Completed = true;
	}
}

bool SystemTestLayer::CheckPipelineAndWorld() {
	bool passed = true;
	const Ayin::Timestep expectedDelta{ 0.25f };
	m_State.Trace.clear();
	m_State.LifecycleTrace.clear();
	m_State.ContextValid = true;
	m_State.ExpectedScene = m_Scene.get();
	m_State.ExpectedDelta = expectedDelta.GetSeconds();

	// 非活动 World 不允许 Update/End，None 也不能作为会话模式。
	if (m_World->Update(expectedDelta) || m_World->EndWorldExecutionSession() ||
		m_World->BeginWorldExecutionSession(Ayin::SceneMode::None)) {
		SetFailure("World accepted an operation in an invalid state");
		passed = false;
	}

	// Begin 按 Order 正序，End 按本次真正 Begin 的 System 逆序；活动会话不能重复 Begin。
	const bool editorBegan = m_World->BeginWorldExecutionSession(Ayin::SceneMode::Editor);
	const bool duplicateBeginRejected = !m_World->BeginWorldExecutionSession(Ayin::SceneMode::Runtime);
	const bool editorUpdated = editorBegan && m_World->Update(expectedDelta);
	const bool editorEnded = editorUpdated && m_World->EndWorldExecutionSession();
	const std::vector<std::string> expectedEditorLifecycle{
		"Early:Begin:Editor", "Late:Begin:Editor",
		"Late:End:Editor", "Early:End:Editor"
	};
	m_State.BeginEndOrderPassed = duplicateBeginRejected &&
		m_State.LifecycleTrace == expectedEditorLifecycle;
	if (!editorBegan || !editorUpdated || !editorEnded || !m_State.BeginEndOrderPassed) {
		SetFailure("World Begin/End lifecycle order mismatch");
		passed = false;
	}

	const auto editorTrace = ExpectedEditorTrace();
	m_State.AutomaticOrderPassed = m_State.Trace == editorTrace;
	m_State.ContextForwardingPassed = m_State.ContextValid;
	if (!m_State.AutomaticOrderPassed || !m_State.ContextForwardingPassed) {
		SetFailure("pipeline phase order or SystemContext forwarding mismatch");
		passed = false;
	}
	if (m_World->Update(expectedDelta)) {
		SetFailure("World::Update accepted an ended session");
		passed = false;
	}

	// Runtime 下只有 RuntimeSystem 可运行。
	m_State.Trace.clear();
	m_State.LifecycleTrace.clear();
	if (!m_World->BeginWorldExecutionSession(Ayin::SceneMode::Runtime) ||
		!m_World->Update(expectedDelta) || !m_World->EndWorldExecutionSession()) {
		SetFailure("World runtime lifecycle failed");
		passed = false;
	}
	const auto runtimeTrace = ExpectedRuntimeTrace();
	const std::vector<std::string> expectedRuntimeLifecycle{
		"Runtime:Begin:Runtime", "Runtime:End:Runtime"
	};
	m_State.ModeFilteringPassed = m_State.Trace == runtimeTrace &&
		m_State.LifecycleTrace == expectedRuntimeLifecycle;
	if (!m_State.ModeFilteringPassed || m_World->EndWorldExecutionSession()) {
		SetFailure("runtime mode filtering or End state mismatch");
		passed = false;
	}

	// TransitionMode 必须先逆序结束旧模式，再正序开始新模式。
	m_State.LifecycleTrace.clear();
	const bool transitionPassed =
		m_World->BeginWorldExecutionSession(Ayin::SceneMode::Editor) &&
		m_World->TransitionMode(Ayin::SceneMode::Runtime) &&
		m_World->EndWorldExecutionSession();
	const std::vector<std::string> expectedTransition{
		"Early:Begin:Editor", "Late:Begin:Editor",
		"Late:End:Editor", "Early:End:Editor",
		"Runtime:Begin:Runtime", "Runtime:End:Runtime"
	};
	m_State.TransitionPassed = transitionPassed && m_State.LifecycleTrace == expectedTransition;
	if (!m_State.TransitionPassed) {
		SetFailure("World::TransitionMode lifecycle mismatch");
		passed = false;
	}

	// Simulation 与 Editor 共用 Early/Late。
	m_State.Trace.clear();
	if (!m_World->BeginWorldExecutionSession(Ayin::SceneMode::Simulation) ||
		!m_World->Update(expectedDelta) || m_State.Trace != editorTrace ||
		!m_World->EndWorldExecutionSession()) {
		SetFailure("simulation mode filtering mismatch");
		passed = false;
	}

	// 空 Scene 不能建立会话，后续 Update/End 也必须保持拒绝。
	Ayin::SystemPipeline::Builder emptyBuilder;
	Ayin::World emptyWorld(nullptr, emptyBuilder.Build());
	if (emptyWorld.BeginWorldExecutionSession(Ayin::SceneMode::Runtime) ||
		emptyWorld.Update(expectedDelta) || emptyWorld.EndWorldExecutionSession()) {
		SetFailure("World accepted a null scene");
		passed = false;
	}

	m_State.WorldLifecyclePassed = passed;
	return passed;
}

bool SystemTestLayer::CheckScheduleLifecycle() {
	Ayin::SystemSchedule schedule;
	// 这一组测试绕过 World，直接验证 Schedule 自己的注册和生命周期接口。
	const Ayin::Timestep expectedDelta{ 0.5f };
	m_State.Trace.clear();
	m_State.ContextValid = true;
	m_State.ExpectedScene = m_Scene.get();
	m_State.ExpectedDelta = expectedDelta.GetSeconds();

	// 第二次添加同一种 System 不应创建第二个实例，也不应再次 OnAttach。
	schedule.AddSystem<LifecycleSystem>({ Ayin::SystemPhase::Update }, { Ayin::SceneMode::Editor });
	schedule.AddSystem<LifecycleSystem>({ Ayin::SystemPhase::Update }, { Ayin::SceneMode::Editor });
	const bool duplicatePassed = schedule.Contain<LifecycleSystem>() && m_State.AttachCount["Lifecycle"] == 1;
	m_State.DuplicateAddPassed = duplicatePassed;
	if (!duplicatePassed) {
		SetFailure("duplicate system registration was not ignored");
	}

	Ayin::SystemContext context{
		.Scene = *m_Scene,
		.DeltaTime = expectedDelta,
		.Mode = Ayin::SceneMode::Editor,
		.Phase = Ayin::SystemPhase::None,
	};
	// LifecycleSystem 只注册 Update，因此轨迹中只能出现一个 Update。
	schedule.Run(context);
	const bool runPassed = m_State.Trace == std::vector<std::string>{ "Lifecycle:Update" } && m_State.ContextValid;
	if (!runPassed) {
		SetFailure("lifecycle system did not run with the expected context");
	}

	// 连续移除两次，验证第二次是安全的无操作，并且 OnDetach 只发生一次。
	schedule.RemoveSystem<LifecycleSystem>();
	schedule.RemoveSystem<LifecycleSystem>();
	const bool removePassed = !schedule.Contain<LifecycleSystem>() && m_State.DetachCount["Lifecycle"] == 1;
	m_State.RemovePassed = removePassed;
	if (!removePassed) {
		SetFailure("system removal or detach lifecycle mismatch");
	}

	// 显式 order 为 -1 的 LateSystem 应排在 order 为 0 的 EarlySystem 前面。
	Ayin::SystemSchedule explicitSchedule;
	m_State.Trace.clear();
	explicitSchedule.AddSystem<LateSystem>({ Ayin::SystemPhase::Update }, { Ayin::SceneMode::Editor }, -1);
	explicitSchedule.AddSystem<EarlySystem>({ Ayin::SystemPhase::Update }, { Ayin::SceneMode::Editor }, 0);
	explicitSchedule.Run(context);
	const bool explicitPassed = m_State.Trace == std::vector<std::string>{ "Late:Update", "Early:Update" };
	m_State.ExplicitOrderPassed = explicitPassed;
	if (!explicitPassed) {
		SetFailure("explicit system order mismatch");
	}
	explicitSchedule.RemoveSystem<LateSystem>();
	explicitSchedule.RemoveSystem<EarlySystem>();

	return duplicatePassed && runPassed && removePassed && explicitPassed;
}

bool SystemTestLayer::CheckDestructorCleanupAndMove() {
	bool passed = true;
	const Ayin::SystemContext editorContext{
		.Scene = *m_Scene,
		.Mode = Ayin::SceneMode::Editor,
	};

	// Schedule 析构必须按注册的相反顺序 Detach。
	m_State.LifecycleTrace.clear();
	{
		Ayin::SystemSchedule schedule;
		schedule.AddSystem<EarlySystem>({}, { Ayin::SceneMode::Editor });
		schedule.AddSystem<LateSystem>({}, { Ayin::SceneMode::Editor });
		schedule.AddSystem<RuntimeSystem>({}, { Ayin::SceneMode::Editor });
	}
	const std::vector<std::string> expectedScheduleDestruction{
		"Early:Attach", "Late:Attach", "Runtime:Attach",
		"Runtime:Detach", "Late:Detach", "Early:Detach"
	};
	const bool scheduleDestructorPassed = m_State.LifecycleTrace == expectedScheduleDestruction;

	// 默认移动构造必须转移所有权；被移动对象析构时不能重复 Detach。
	m_State.LifecycleTrace.clear();
	{
		Ayin::SystemSchedule source;
		source.AddSystem<EarlySystem>({}, { Ayin::SceneMode::Editor });
		source.AddSystem<LateSystem>({}, { Ayin::SceneMode::Editor });
		Ayin::SystemSchedule destination{ std::move(source) };
		destination.Begin(editorContext);
		destination.End(editorContext);
	}
	const std::vector<std::string> expectedMoveLifecycle{
		"Early:Attach", "Late:Attach",
		"Early:Begin:Editor", "Late:Begin:Editor",
		"Late:End:Editor", "Early:End:Editor",
		"Late:Detach", "Early:Detach"
	};
	m_State.MoveConstructionPassed = m_State.LifecycleTrace == expectedMoveLifecycle;

	// World 在活动会话中析构时，先兜底 End，再由 Schedule 逆序 Detach。
	m_State.LifecycleTrace.clear();
	{
		Ayin::World world{ m_Scene, m_Pipeline };
		if (!world.BeginWorldExecutionSession(Ayin::SceneMode::Editor)) {
			SetFailure("temporary World could not begin");
			passed = false;
		}
	}
	const std::vector<std::string> expectedWorldDestruction{
		"Early:Attach", "Late:Attach", "Runtime:Attach",
		"Early:Begin:Editor", "Late:Begin:Editor",
		"Late:End:Editor", "Early:End:Editor",
		"Runtime:Detach", "Late:Detach", "Early:Detach"
	};
	const bool worldDestructorPassed = m_State.LifecycleTrace == expectedWorldDestruction;
	m_State.DestructorCleanupPassed = scheduleDestructorPassed && worldDestructorPassed;

	if (!m_State.DestructorCleanupPassed) {
		SetFailure("Schedule or World destructor cleanup order mismatch");
		passed = false;
	}
	if (!m_State.MoveConstructionPassed) {
		SetFailure("SystemSchedule move construction duplicated or lost lifecycle callbacks");
		passed = false;
	}

	return passed;
}

void SystemTestLayer::RunLiveFrame(const Ayin::Timestep deltaTime) {
	// 这是对主循环路径的最终验证：使用真实帧时间调用 World 一次完整生命周期。
	m_State.Trace.clear();
	m_State.ContextValid = true;
	m_State.ExpectedScene = m_Scene.get();
	m_State.ExpectedDelta = deltaTime.GetSeconds();

	const bool began = m_World->BeginWorldExecutionSession(Ayin::SceneMode::Editor);
	const bool updated = began && m_World->Update(deltaTime);
	const bool ended = updated && m_World->EndWorldExecutionSession();
	const bool tracePassed = m_State.Trace == ExpectedEditorTrace();

	if (!began || !updated || !ended || !tracePassed || !m_State.ContextValid) {
		SetFailure("live World frame did not produce the expected result");
		return;
	}

	m_State.LiveFramePassed = true;
	m_State.Completed = true;
	m_State.Pass = true;
	// 即使应用在退出前又执行一帧，也只输出一次最终结果。
	if (!m_LoggedResult) {
		AYIN_INFO("SYSTEM_WORLD_TEST: PASS");
		m_LoggedResult = true;
	}
}

void SystemTestLayer::SetFailure(const char* failure) {
	// 保留第一条失败原因，后续失败仍会让测试保持失败，但不会覆盖最有用的诊断信息。
	if (m_State.Failure.empty()) {
		m_State.Failure = failure;
	}
	m_State.Pass = false;
	if (!m_LoggedResult) {
		AYIN_ERROR("SYSTEM_WORLD_TEST: FAIL - {0}", m_State.Failure);
		m_LoggedResult = true;
	}
}

void SystemTestLayer::RenderCheck(const char* label, const bool passed) const {
	ImGui::Text("%s: %s", label, passed ? "pass" : "fail");
}

std::vector<std::string> SystemTestLayer::ExpectedEditorTrace() const {
	std::vector<std::string> expected;
	// 调度器先完成一个阶段内的所有 System，再进入下一个阶段。
	for (const char* phase : kPhases) {
		expected.emplace_back("Early:");
		expected.back() += phase;
		expected.emplace_back("Late:");
		expected.back() += phase;
	}
	return expected;
}

std::vector<std::string> SystemTestLayer::ExpectedRuntimeTrace() const {
	return { "Runtime:PreUpdate", "Runtime:Update", "Runtime:PostUpdate", "Runtime:Presentation" };
}

const char* SystemTestLayer::ResultText() const {
	return m_State.Pass ? "PASS" : (m_State.Failure.empty() ? "PENDING" : "FAIL");
}

void SystemTestLayer::OnImGuiRender() {
	// ImGui 面板用于手动观察；自动化验证则依赖日志中的 PASS/FAIL 标记。
	ImGui::Begin("World and System Tests");
	ImGui::Text("Result: %s", ResultText());
	ImGui::Text("Auto exit: %s", m_AutoExitEnabled ? "enabled" : "disabled");
	ImGui::Text("Exit requested: %s", m_AutoExitRequested ? "yes" : "no");
	RenderCheck("Pipeline built", m_State.PipelineBuilt);
	RenderCheck("Automatic order", m_State.AutomaticOrderPassed);
	RenderCheck("Explicit order", m_State.ExplicitOrderPassed);
	RenderCheck("Mode filtering", m_State.ModeFilteringPassed);
	RenderCheck("Context forwarding", m_State.ContextForwardingPassed);
	RenderCheck("World lifecycle", m_State.WorldLifecyclePassed);
	RenderCheck("Duplicate add", m_State.DuplicateAddPassed);
	RenderCheck("Remove and detach", m_State.RemovePassed);
	RenderCheck("Attach registration order", m_State.AttachOrderPassed);
	RenderCheck("Begin/End reverse order", m_State.BeginEndOrderPassed);
	RenderCheck("Mode transition", m_State.TransitionPassed);
	RenderCheck("Destructor fallback cleanup", m_State.DestructorCleanupPassed);
	RenderCheck("Schedule move construction", m_State.MoveConstructionPassed);
	RenderCheck("Live World frame", m_State.LiveFramePassed);
	if (!m_State.Failure.empty()) {
		ImGui::Separator();
		ImGui::Text("Failure: %s", m_State.Failure.c_str());
	}
	ImGui::Separator();
	ImGui::Text("Trace:");
	for (const auto& entry : m_State.Trace) {
		ImGui::BulletText("%s", entry.c_str());
	}
	ImGui::End();
}

void SystemTestLayer::ProbeSystem::Bind(TestState* state) {
	s_State = state;
}

void SystemTestLayer::ProbeSystem::Unbind() {
	s_State = nullptr;
}

void SystemTestLayer::ProbeSystem::OnAttach() {
	if (s_State != nullptr) {
		++s_State->AttachCount[Name()];
		s_State->LifecycleTrace.emplace_back(std::string{ Name() } + ":Attach");
	}
}

void SystemTestLayer::ProbeSystem::OnDetach() {
	if (s_State != nullptr) {
		++s_State->DetachCount[Name()];
		s_State->LifecycleTrace.emplace_back(std::string{ Name() } + ":Detach");
	}
}

void SystemTestLayer::ProbeSystem::OnBegin(const Ayin::SystemContext& context) {
	if (s_State != nullptr) {
		if (&context.Scene != s_State->ExpectedScene || context.Phase != Ayin::SystemPhase::None) {
			s_State->ContextValid = false;
		}
		s_State->LifecycleTrace.emplace_back(
			std::string{ Name() } + ":Begin:" + ModeName(context.Mode));
	}
}

void SystemTestLayer::ProbeSystem::OnEnd(const Ayin::SystemContext& context) {
	if (s_State != nullptr) {
		if (&context.Scene != s_State->ExpectedScene || context.Phase != Ayin::SystemPhase::None) {
			s_State->ContextValid = false;
		}
		s_State->LifecycleTrace.emplace_back(
			std::string{ Name() } + ":End:" + ModeName(context.Mode));
	}
}

void SystemTestLayer::ProbeSystem::RecordPhase(const Ayin::SystemContext& context, const char* phase) {
	if (s_State == nullptr) {
		return;
	}

	// 同时检查场景、DeltaTime 和 Phase，防止只验证“回调被调用”而漏掉上下文错误。
	if (!(&context.Scene == s_State->ExpectedScene &&
		std::fabs(context.DeltaTime.GetSeconds() - s_State->ExpectedDelta) < 0.0001f &&
		IsPhase(context.Phase, phase))) {
		s_State->ContextValid = false;
	}

	s_State->Trace.emplace_back(std::string{ Name() } + ":" + phase);
}
