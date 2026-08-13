#include <AyinPch.h>

#include "TestLayer.h"

#include <Ayin/Core/Application.h>

#include <cmath>
#include <cstdlib>
#include <imgui.h>

namespace {
	constexpr const char* kAutoExitEnv = "AYIN_SANDBOX_SMOKE_AUTO_EXIT";
	// SystemSchedule 固定按这四个阶段执行；测试轨迹也使用相同顺序。
	constexpr std::array<const char*, 4> kPhases{ "PreUpdate", "Update", "PostUpdate", "Presentation" };

	// 环境变量不存在、为空或值为 0 时关闭自动退出，其他非空值都视为开启。
	bool IsAutoExitEnabled(const char* value) {
		return value != nullptr && value[0] != '\0' && value[0] != '0';
	}

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
}

TestLayer::TestState* TestLayer::ProbeSystem::s_State = nullptr;

TestLayer::TestLayer()
	: Ayin::Layer("TestLayer") {
	// getenv 只读取当前进程环境，不会修改系统环境变量。
	m_AutoExitEnabled = IsAutoExitEnabled(std::getenv(kAutoExitEnv));
}

TestLayer::~TestLayer() = default;

void TestLayer::OnAttach() {
	// 先绑定观测状态，再创建 World，确保 Pipeline 中的 System 在 OnAttach
	// 时就能把生命周期数据写入当前 TestLayer。
	ProbeSystem::Bind(&m_State);
	BuildWorld();
	RunOneShotChecks();
}

void TestLayer::OnDetach() {
	// World 当前只负责销毁 Schedule，测试用例中的显式移除已经验证了 OnDetach。
	m_World.reset();
	ProbeSystem::Unbind();
}

void TestLayer::OnUpdate(const Ayin::Timestep deltaTime) {
	// 一次性检查通常已经在 OnAttach 完成；这里保留补偿调用，避免初始化顺序变化时漏测。
	if (!m_RanChecks) {
		RunOneShotChecks();
	}

	if (m_RanChecks && !m_State.LiveFramePassed) {
		// 通过真实应用帧调用 World，而不是直接调用 Schedule，验证最终使用路径。
		RunLiveFrame(deltaTime);
	}

	if (m_AutoExitEnabled && m_State.LiveFramePassed && !m_AutoExitRequested) {
		// 自动退出只用于命令行烟雾测试；普通运行时不设置环境变量即可保持窗口打开。
		m_AutoExitRequested = true;
		Ayin::Application::Get().Close();
	}
}

void TestLayer::BuildWorld() {
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
}

void TestLayer::RunOneShotChecks() {
	if (m_RanChecks || !m_State.PipelineBuilt || m_World == nullptr) {
		return;
	}

	const bool pipelinePassed = CheckPipelineAndWorld();
	const bool schedulePassed = CheckScheduleLifecycle();
	// 两组测试都结束且没有失败信息后，下一帧才进入真实帧测试。
	if (pipelinePassed && schedulePassed && m_State.Failure.empty()) {
		m_RanChecks = true;
	}
}

bool TestLayer::CheckPipelineAndWorld() {
	bool passed = true;
	// 使用固定 DeltaTime，便于确认 SystemContext 没有丢失或篡改时间数据。
	const Ayin::Timestep expectedDelta{ 0.25f };
	m_State.Trace.clear();
	m_State.ContextValid = true;
	m_State.ExpectedScene = m_Scene.get();
	m_State.ExpectedDelta = expectedDelta.GetSeconds();

	// 未 Begin 时 Update 必须被拒绝。
	if (m_World->Update(expectedDelta)) {
		SetFailure("World::Update accepted an inactive world");
		passed = false;
	}
	// None 不是可执行模式，Begin 应返回 false 且不改变 World 状态。
	if (m_World->Begin(Ayin::SceneMode::None)) {
		SetFailure("World::Begin accepted SceneMode::None");
		passed = false;
	}
	// Editor 模式执行后，Early/Late 应在每个阶段各运行一次。
	if (!m_World->Begin(Ayin::SceneMode::Editor) || !m_World->Update(expectedDelta)) {
		SetFailure("World editor lifecycle did not begin and update");
		passed = false;
	}

	// 轨迹是按“阶段 -> 阶段内的系统顺序”排列，而不是按 System 分组。
	const auto editorTrace = ExpectedEditorTrace();
	if (m_State.Trace != editorTrace) {
		SetFailure("pipeline automatic order or editor phases mismatch");
		passed = false;
	}
	m_State.AutomaticOrderPassed = m_State.Trace == editorTrace;
	m_State.ContextForwardingPassed = m_State.ContextValid;
	if (!m_State.ContextValid) {
		SetFailure("SystemContext was not forwarded correctly");
		passed = false;
	}

	// End 后再次 Update 必须被拒绝，证明当前模式已回到 None。
	if (!m_World->End() || m_World->Update(expectedDelta)) {
		SetFailure("World end or post-end update behavior mismatch");
		passed = false;
	}

	// Runtime 下 Early/Late 不应运行，只有 RuntimeSystem 应留下轨迹。
	m_State.Trace.clear();
	if (!m_World->Begin(Ayin::SceneMode::Runtime) || !m_World->Update(expectedDelta)) {
		SetFailure("World runtime lifecycle did not begin and update");
		passed = false;
	}

	const auto runtimeTrace = ExpectedRuntimeTrace();
	if (m_State.Trace != runtimeTrace) {
		SetFailure("runtime mode filtering mismatch");
		passed = false;
	}
	m_State.ModeFilteringPassed = m_State.Trace == runtimeTrace;

	// 第一次 End 成功，第二次 End 应报告非法状态。
	if (!m_World->End() || m_World->End()) {
		SetFailure("World end state transition mismatch");
		passed = false;
	}

	// Simulation 与 Editor 共用 Early/Late，验证第二种可执行模式没有被遗漏。
	m_State.Trace.clear();
	if (!m_World->Begin(Ayin::SceneMode::Simulation) || !m_World->Update(expectedDelta)) {
		SetFailure("World simulation lifecycle did not begin and update");
		passed = false;
	}
	if (m_State.Trace != editorTrace) {
		SetFailure("simulation mode filtering mismatch");
		passed = false;
	}
	if (!m_World->End()) {
		SetFailure("World simulation end failed");
		passed = false;
	}

	// 空场景仍可 Begin，但 Update 必须拒绝解引用空场景。
	Ayin::SystemPipeline::Builder emptyBuilder;
	Ayin::World emptyWorld(nullptr, emptyBuilder.Build());
	if (!emptyWorld.Begin(Ayin::SceneMode::Runtime) || emptyWorld.Update(expectedDelta) || !emptyWorld.End()) {
		SetFailure("World did not reject a null scene");
		passed = false;
	}

	m_State.WorldLifecyclePassed = passed;

	return passed;
}

bool TestLayer::CheckScheduleLifecycle() {
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

void TestLayer::RunLiveFrame(const Ayin::Timestep deltaTime) {
	// 这是对主循环路径的最终验证：使用真实帧时间调用 World 一次完整生命周期。
	m_State.Trace.clear();
	m_State.ContextValid = true;
	m_State.ExpectedScene = m_Scene.get();
	m_State.ExpectedDelta = deltaTime.GetSeconds();

	const bool began = m_World->Begin(Ayin::SceneMode::Editor);
	const bool updated = began && m_World->Update(deltaTime);
	const bool ended = updated && m_World->End();
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

void TestLayer::SetFailure(const char* failure) {
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

void TestLayer::RenderCheck(const char* label, const bool passed) const {
	ImGui::Text("%s: %s", label, passed ? "pass" : "fail");
}

std::vector<std::string> TestLayer::ExpectedEditorTrace() const {
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

std::vector<std::string> TestLayer::ExpectedRuntimeTrace() const {
	return { "Runtime:PreUpdate", "Runtime:Update", "Runtime:PostUpdate", "Runtime:Presentation" };
}

const char* TestLayer::ResultText() const {
	return m_State.Pass ? "PASS" : (m_State.Failure.empty() ? "PENDING" : "FAIL");
}

void TestLayer::OnImGuiRender() {
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

void TestLayer::ProbeSystem::Bind(TestState* state) {
	s_State = state;
}

void TestLayer::ProbeSystem::Unbind() {
	s_State = nullptr;
}

void TestLayer::ProbeSystem::OnAttach() {
	// Schedule 创建 System 后立即调用 OnAttach，因此这里能验证实例确实被创建。
	if (s_State != nullptr) {
		++s_State->AttachCount[Name()];
	}
}

void TestLayer::ProbeSystem::OnDetach() {
	// 显式 RemoveSystem 时调用 OnDetach；World 销毁路径目前不作为本测试的生命周期断言。
	if (s_State != nullptr) {
		++s_State->DetachCount[Name()];
	}
}

void TestLayer::ProbeSystem::RecordPhase(const Ayin::SystemContext& context, const char* phase) {
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
