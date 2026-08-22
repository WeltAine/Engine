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
	RegisterTestSystems();
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

void SystemTestLayer::RegisterTestSystems() {
	// Registry 是进程级静态存储；同一个 SandBox 进程内即使重建测试 Layer，
	// 也不能把相同类型重复注册进去。
	static bool registered = false;
	if (registered) {
		return;
	}

	Ayin::SystemRegistry::Register<EarlySystem>("SandBox.Tests.EarlySystem", "Early System", {}, {}, 0);
	Ayin::SystemRegistry::Register<LateSystem>("SandBox.Tests.LateSystem", "Late System", {}, {}, 0);
	Ayin::SystemRegistry::Register<RuntimeSystem>("SandBox.Tests.RuntimeSystem", "Runtime System", {}, {}, 0);
	Ayin::SystemRegistry::Register<LifecycleSystem>("SandBox.Tests.LifecycleSystem", "Lifecycle System", {}, {}, 0);
	Ayin::SystemRegistry::Register<FailingAttachSystem>("SandBox.Tests.FailingAttachSystem", "Failing Attach System", {}, {}, 0);
	Ayin::SystemRegistry::Register<SerializationSystem>("SandBox.Tests.SerializationSystem", "Serialization System", {}, {}, 0);
	registered = true;
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
	const bool baselinePassed = CheckScheduleBaseline();
	const bool builderModelPassed = CheckPipelineBuilder();
	const bool registryPassed = CheckSystemRegistry();
	const bool serializationPassed = CheckSystemSerialization();
	const bool editorInteractionPassed = CheckEditorInteractionBoundaries();
	m_RanChecks = true;

	// 一次性检查失败时也标记完成，使自动化运行能够退出并报告 FAIL，而不是一直挂起窗口。
	if (!pipelinePassed || !schedulePassed || !cleanupPassed || !baselinePassed || !builderModelPassed || !registryPassed || !serializationPassed || !m_State.SerializationFailurePassed || !editorInteractionPassed || !m_State.ApplyFailurePassed || !m_State.SystemObservationPassed || !m_State.Failure.empty()) {
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

	// Editor 通过 World 的通用实例访问自行选择 System；World 不负责包装 GUI 调用。
	const std::size_t lifecycleTraceSizeBeforeGui = m_State.LifecycleTrace.size();
	Ayin::ISystem* selectedSystem = m_World->FindSystemInstance(Ayin::GetSystemID<EarlySystem>());
	const Ayin::World& constWorld = *m_World;
	const bool worldSystemAccessPassed = selectedSystem != nullptr &&
		m_World->FindSystemInstance("SandBox.Tests.EarlySystem") == selectedSystem &&
		constWorld.FindSystemInstance(Ayin::GetSystemID<EarlySystem>()) != nullptr;

	// Schedule 只暴露 const 条目；Editor 可以读取名称、规格和实例，但不通过此接口改写拓扑。
	const Ayin::SystemSchedule& observedSchedule = constWorld.GetSystemSchedule();
	const std::vector<Ayin::SystemEntry>& observedSystems = observedSchedule.GetSystems();
	const Ayin::SystemEntry* observedEarly = observedSchedule.FindSystemEntry(Ayin::GetSystemID<EarlySystem>());
	m_State.SystemObservationPassed = observedSystems.size() == 3 &&
		observedEarly != nullptr &&
		observedEarly->Information.TypeKey == "SandBox.Tests.EarlySystem" &&
		observedEarly->Specification.Order == 0 &&
		observedEarly->GetInstance() == selectedSystem;
	if (!m_State.SystemObservationPassed) {
		SetFailure("SystemSchedule read-only observation mismatch");
		passed = false;
	}

	if (selectedSystem != nullptr)
		selectedSystem->OnEditorGui();
	m_State.EditorGuiPassed = worldSystemAccessPassed &&
		m_State.EditorGuiCount["Early"] == 1 &&
		m_State.LifecycleTrace.size() == lifecycleTraceSizeBeforeGui;
	if (!m_State.EditorGuiPassed) {
		SetFailure("World system access or direct System editor GUI invocation failed");
		passed = false;
	}

	// 非活动 World 不允许 Update；重复 End 是幂等空操作，None 也不能作为会话模式。
	if (m_World->Update(expectedDelta) ||
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
	const bool repeatedEndNoOp = m_World->EndWorldExecutionSession() && !m_World->SessionReady();
	if (!m_State.ModeFilteringPassed || !repeatedEndNoOp) {
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

	// Apply 先完整构建候选 Schedule，再结束旧 Schedule，并恢复原有 Editor 会话。
	m_State.LifecycleTrace.clear();
	Ayin::SystemPipeline::Builder replacementBuilder;
	replacementBuilder.AddSystem<EarlySystem>(
		{ Ayin::SystemPhase::Update },
		{ Ayin::SceneMode::Editor });
	const Ayin::SystemPipeline replacementPipeline = replacementBuilder.Build();
	const std::size_t attachTraceSizeBeforeCandidate = m_State.LifecycleTrace.size();
	Ayin::SystemSchedule detachedCandidate = replacementPipeline.CreateSchedule();
	const bool candidateConstructionPassed =
		detachedCandidate.IsBuilt() && !detachedCandidate.IsAttached() &&
		m_State.LifecycleTrace.size() == attachTraceSizeBeforeCandidate;
	Ayin::World applyWorld{ m_Scene, m_Pipeline };
	const bool applyStarted = applyWorld.BeginWorldExecutionSession(Ayin::SceneMode::Editor);
	const bool applySucceeded = applyStarted && applyWorld.ApplyPipeline(replacementPipeline);
	const std::vector<std::string> expectedApplyLifecycle{
		"Early:Attach", "Late:Attach", "Runtime:Attach",
		"Early:Begin:Editor", "Late:Begin:Editor", "Late:End:Editor", "Early:End:Editor", "Runtime:Detach", "Late:Detach", "Early:Detach",
		"Early:Attach", "Early:Begin:Editor", "Early:End:Editor"
	};
	const bool applyEnded = applySucceeded && applyWorld.EndWorldExecutionSession();
	m_State.ApplyPassed = candidateConstructionPassed && applyEnded && m_State.LifecycleTrace == expectedApplyLifecycle;
	if (!m_State.ApplyPassed)
		SetFailure("World Apply lifecycle replacement mismatch");
	// 候选 Schedule 构建失败时，旧 World 不应结束当前会话或替换 System 实例。
	Ayin::SystemPipeline::Builder invalidBuilder;
	invalidBuilder.AddSystem(Ayin::SystemDefinition{
		.Type{"SandBox.Tests.SerializationSystem"},
		.Specification{.PhaseMask{Ayin::SystemPhase::Update}, .ModeMask{Ayin::SceneMode::Editor}},
		.Configuration{.Json{"{\"Exposure\":"}}
	});
	const Ayin::SystemPipeline invalidPipeline = invalidBuilder.Build();
	Ayin::World invalidApplyWorld{ m_Scene, m_Pipeline };
	const Ayin::SystemID originalSystemId = Ayin::GetSystemID<EarlySystem>();
	Ayin::ISystem* originalSystem = invalidApplyWorld.FindSystemInstance(originalSystemId);
	const bool invalidApplyStarted = invalidApplyWorld.BeginWorldExecutionSession(Ayin::SceneMode::Editor);
	m_State.LifecycleTrace.clear();
	const bool invalidApplyRejected = invalidApplyStarted && !invalidApplyWorld.ApplyPipeline(invalidPipeline);
	const bool invalidApplyKeptWorld = invalidApplyWorld.GetCurrentMode() == Ayin::SceneMode::Editor &&
		invalidApplyWorld.GetSystemSchedule().IsActive() &&
		invalidApplyWorld.FindSystemInstance(originalSystemId) == originalSystem &&
		m_State.LifecycleTrace.empty();
	invalidApplyWorld.EndWorldExecutionSession();

	// 候选 Schedule Attach 失败时，World 回滚到旧 Schedule；失败的候选不会泄漏生命周期状态。
	Ayin::SystemPipeline::Builder failingBuilder;
	failingBuilder.AddSystem<FailingAttachSystem>({}, { Ayin::SceneMode::Editor });
	const Ayin::SystemPipeline failingPipeline = failingBuilder.Build();
	Ayin::World failingApplyWorld{ m_Scene, m_Pipeline };
	Ayin::ISystem* failingOriginalSystem = failingApplyWorld.FindSystemInstance(originalSystemId);
	const bool failingApplyStarted = failingApplyWorld.BeginWorldExecutionSession(Ayin::SceneMode::Editor);
	m_State.LifecycleTrace.clear();
	const bool failingApplyRejected = failingApplyStarted && !failingApplyWorld.ApplyPipeline(failingPipeline);
	const bool failingApplyRolledBack = failingApplyWorld.GetCurrentMode() == Ayin::SceneMode::Editor &&
		failingApplyWorld.GetSystemSchedule().IsActive() &&
		failingApplyWorld.FindSystemInstance(originalSystemId) == failingOriginalSystem;
	failingApplyWorld.EndWorldExecutionSession();
	m_State.ApplyFailurePassed = invalidApplyRejected && invalidApplyKeptWorld &&
		failingApplyRejected && failingApplyRolledBack;
	if (!m_State.ApplyFailurePassed) {
		AYIN_CORE_ERROR("Apply failure checks: invalidRejected={}, invalidKept={}, failingRejected={}, rollback={}", invalidApplyRejected, invalidApplyKeptWorld, failingApplyRejected, failingApplyRolledBack);
		SetFailure("World Apply failure boundary mismatch");
	}

	// 空 Scene 不能建立会话，后续 Update/End 也必须保持拒绝。
	Ayin::SystemPipeline::Builder emptyBuilder;
	Ayin::World emptyWorld(nullptr, emptyBuilder.Build());
	if (emptyWorld.BeginWorldExecutionSession(Ayin::SceneMode::Runtime) ||
		emptyWorld.Update(expectedDelta) || !emptyWorld.EndWorldExecutionSession()) {
		SetFailure("World accepted a null scene");
		passed = false;
	}

	m_State.WorldLifecyclePassed = passed;
	return passed;
}

bool SystemTestLayer::CheckScheduleLifecycle() {
	// 这一组测试通过 World 验证 Pipeline 创建出的 Schedule 生命周期。
	const Ayin::Timestep expectedDelta{ 0.5f };
	m_State.Trace.clear();
	m_State.ContextValid = true;
	m_State.ExpectedScene = m_Scene.get();
	m_State.ExpectedDelta = expectedDelta.GetSeconds();

	Ayin::SystemPipeline::Builder lifecycleBuilder;
	lifecycleBuilder.AddSystem<LifecycleSystem>(
		{ Ayin::SystemPhase::Update },
		{ Ayin::SceneMode::Editor });
	const Ayin::SystemPipeline lifecyclePipeline = lifecycleBuilder.Build();
	Ayin::World lifecycleWorld{ m_Scene, lifecyclePipeline };
	const Ayin::SystemSchedule& schedule = lifecycleWorld.GetSystemSchedule();

	const bool builtPassed = schedule.IsBuilt() && schedule.IsAttached() &&
		schedule.FindSystemInstance(Ayin::GetSystemID<LifecycleSystem>()) != nullptr &&
		m_State.AttachCount["Lifecycle"] == 1;
	m_State.DuplicateAddPassed = builtPassed;
	if (!builtPassed)
		SetFailure("World did not attach the Pipeline-created Lifecycle Schedule");

	// World 只有在 Begin 后才会驱动 Schedule 执行阶段回调。
	const bool runBeforeBeginRejected = !lifecycleWorld.Update(expectedDelta);
	const bool began = lifecycleWorld.BeginWorldExecutionSession(Ayin::SceneMode::Editor);
	const bool duplicateBeginRejected = !lifecycleWorld.BeginWorldExecutionSession(Ayin::SceneMode::Runtime);
	const bool updated = began && lifecycleWorld.Update(expectedDelta);
	const bool ended = updated && lifecycleWorld.EndWorldExecutionSession();
	const bool duplicateEndNoOp = lifecycleWorld.EndWorldExecutionSession();

	const bool lifecyclePassed =
		runBeforeBeginRejected && duplicateBeginRejected && duplicateEndNoOp &&
		m_State.Trace == std::vector<std::string>{ "Lifecycle:Update" } &&
		m_State.BeginCount["Lifecycle"] == 1 &&
		m_State.EndCount["Lifecycle"] == 1 &&
		m_State.ContextValid;
	if (!lifecyclePassed)
		SetFailure("World lifecycle state machine did not enforce Begin/Run/End rules");

	Ayin::SystemPipeline::Builder explicitBuilder;
	explicitBuilder
		.AddSystem<LateSystem>({ Ayin::SystemPhase::Update }, { Ayin::SceneMode::Editor }, -1)
		.AddSystem<EarlySystem>({ Ayin::SystemPhase::Update }, { Ayin::SceneMode::Editor }, 0);
	Ayin::World explicitWorld{ m_Scene, explicitBuilder.Build() };
	m_State.Trace.clear();
	const bool explicitPassed =
		explicitWorld.BeginWorldExecutionSession(Ayin::SceneMode::Editor) &&
		explicitWorld.Update(expectedDelta) &&
		explicitWorld.EndWorldExecutionSession() &&
		m_State.Trace == std::vector<std::string>{ "Late:Update", "Early:Update" };
	m_State.ExplicitOrderPassed = explicitPassed;
	if (!explicitPassed)
		SetFailure("explicit system order mismatch");

	m_State.RemovePassed = lifecyclePassed;
	return builtPassed && lifecyclePassed && explicitPassed;
}
bool SystemTestLayer::CheckDestructorCleanupAndMove() {
	bool passed = true;
	// Pipeline 创建出的裸 Schedule 不会自动 Attach，也就不应在析构时伪造 Detach。
	m_State.LifecycleTrace.clear();
	bool scheduleConstructionPassed = false;
	{
		Ayin::SystemPipeline::Builder builder;
		builder
			.AddSystem<EarlySystem>({}, { Ayin::SceneMode::Editor })
			.AddSystem<LateSystem>({}, { Ayin::SceneMode::Editor })
			.AddSystem<RuntimeSystem>({}, { Ayin::SceneMode::Editor });
		Ayin::SystemSchedule schedule = builder.Build().CreateSchedule();
		scheduleConstructionPassed = schedule.IsBuilt() && !schedule.IsAttached();
	}
	const bool scheduleDestructorPassed = scheduleConstructionPassed && m_State.LifecycleTrace.empty();

	// 移动构造必须转移已构建 Schedule 的所有权；未 Attach 的对象不应产生生命周期回调。
	m_State.LifecycleTrace.clear();
	bool moveConstructionPassed = false;
	{
		Ayin::SystemPipeline::Builder builder;
		builder
			.AddSystem<EarlySystem>({}, { Ayin::SceneMode::Editor })
			.AddSystem<LateSystem>({}, { Ayin::SceneMode::Editor });
		Ayin::SystemSchedule source = builder.Build().CreateSchedule();
		Ayin::SystemSchedule destination{ std::move(source) };
		moveConstructionPassed = destination.IsBuilt() && !destination.IsAttached() &&
			destination.FindSystemInstance(Ayin::GetSystemID<EarlySystem>()) != nullptr;
	}
	m_State.MoveConstructionPassed = moveConstructionPassed && m_State.LifecycleTrace.empty();

	// World 在活动会话中析构时，World 自己先 End，再由 Schedule 逆序 Detach。
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
bool SystemTestLayer::CheckScheduleBaseline() {
	// Schedule 拓扑由 Pipeline 构建并冻结；生命周期由 World 控制调用时机。
	Ayin::SystemPipeline::Builder builder;
	builder.AddSystem<LifecycleSystem>({ Ayin::SystemPhase::Update }, { Ayin::SceneMode::Editor });
	Ayin::World world{ m_Scene, builder.Build() };
	const Ayin::Timestep expectedDelta{ 0.75f };
	m_State.Trace.clear();
	m_State.LifecycleTrace.clear();
	m_State.ContextValid = true;
	m_State.ExpectedScene = m_Scene.get();
	m_State.ExpectedDelta = expectedDelta.GetSeconds();

	const int beginBefore = m_State.BeginCount["Lifecycle"];
	const int endBefore = m_State.EndCount["Lifecycle"];

	// Begin 前 Update 不得执行；重复 Begin / End 都不得重复回调。
	const bool runBeforeBeginRejected = !world.Update(expectedDelta);
	world.BeginWorldExecutionSession(Ayin::SceneMode::Editor);
	world.BeginWorldExecutionSession(Ayin::SceneMode::Runtime);
	world.Update(expectedDelta);
	world.EndWorldExecutionSession();
	world.EndWorldExecutionSession();

	const bool lifecycleStatePassed =
		runBeforeBeginRejected &&
		m_State.Trace == std::vector<std::string>{ "Lifecycle:Update" } &&
		m_State.BeginCount["Lifecycle"] == beginBefore + 1 &&
		m_State.EndCount["Lifecycle"] == endBefore + 1;

	m_State.ScheduleBaselinePassed = lifecycleStatePassed && m_State.ContextValid;
	if (!m_State.ScheduleBaselinePassed)
		SetFailure("SystemSchedule lifecycle state machine mismatch");

	return m_State.ScheduleBaselinePassed;
}
bool SystemTestLayer::CheckPipelineBuilder() {
	// Builder 使用 vector 保存可编辑定义：同 Order 的相对顺序由 stable_sort 保留，唯一性单独检查。
	Ayin::SystemPipeline::Builder builder;
	builder
		.AddSystem(Ayin::SystemDefinition{
			.Type{"SandBox.Tests.LateSystem"},
			.Specification{.PhaseMask{Ayin::SystemPhase::Update}, .ModeMask{Ayin::SceneMode::Editor}, .Order{5}}
		})
		.AddSystem(Ayin::SystemDefinition{
			.Type{"SandBox.Tests.EarlySystem"},
			.Specification{.PhaseMask{Ayin::SystemPhase::Update}, .ModeMask{Ayin::SceneMode::Editor}, .Order{5}}
		})
		.AddSystem(Ayin::SystemDefinition{
			.Type{"SandBox.Tests.SerializationSystem"},
			.Specification{.PhaseMask{Ayin::SystemPhase::Update}, .ModeMask{Ayin::SceneMode::Editor}, .Order{2}},
			.Configuration{.Json{"{\"Exposure\":17}"}}
		});

	// 同一个 TypeKey 不得插入第二次；不依赖容器比较器的副作用表达唯一性。
	builder.AddSystem(Ayin::SystemDefinition{
		.Type{"SandBox.Tests.EarlySystem"},
		.Specification{.PhaseMask{Ayin::SystemPhase::Presentation}, .ModeMask{Ayin::SceneMode::Runtime}, .Order{0}}
	});

	const auto& definitions = builder.GetDefinitions();
	const bool builderOrderPassed = definitions.size() == 3 &&
		definitions[0].Type == "SandBox.Tests.SerializationSystem" && definitions[0].Specification.Order == 2 &&
		definitions[1].Type == "SandBox.Tests.LateSystem" && definitions[1].Specification.Order == 5 &&
		definitions[2].Type == "SandBox.Tests.EarlySystem" && definitions[2].Specification.Order == 5 &&
		definitions[0].Configuration.Json == "{\"Exposure\":17}";

	Ayin::SystemPipeline pipeline = builder.Build();
	const auto& pipelineDefinitions = pipeline.GetDefinitions();
	Ayin::SystemSchedule schedule = pipeline.CreateSchedule();
	const Ayin::SystemEntry* serializedEntry =
		schedule.FindSystemEntry(Ayin::GetSystemID<SerializationSystem>());
	const SerializationSystem* serializedSystem = serializedEntry == nullptr || serializedEntry->GetInstance() == nullptr
		? nullptr
		: static_cast<const SerializationSystem*>(schedule.FindSystemInstance(Ayin::GetSystemID<SerializationSystem>()));
	const bool pipelinePassed = pipelineDefinitions.size() == 3 &&
		pipelineDefinitions[0].Type == definitions[0].Type &&
		pipelineDefinitions[1].Type == definitions[1].Type &&
		pipelineDefinitions[2].Type == definitions[2].Type &&
		serializedSystem != nullptr && serializedSystem->Exposure == 17;


	// Build 只读取 Builder 的当前快照；之后继续修改 Builder 不会反向改变已经生成的 Pipeline。
	builder.SetSystemConfiguration(
		Ayin::GetSystemID<SerializationSystem>(),
		Ayin::SystemConfiguration{.Json{"{\"Exposure\":29}"}});
	const Ayin::SystemPipeline editedPipeline = builder.Build();
	const bool builderRemainsEditable =
		pipelineDefinitions[0].Configuration.Json == "{\"Exposure\":17}" &&
		editedPipeline.GetDefinitions()[0].Configuration.Json == "{\"Exposure\":29}";
	m_State.BuilderModelPassed = builderOrderPassed && pipelinePassed && builderRemainsEditable;
	if (!m_State.BuilderModelPassed) {
		SetFailure("Pipeline Builder did not preserve definition order or configuration");
	}

	return m_State.BuilderModelPassed;
}

bool SystemTestLayer::CheckSystemRegistry() {
	// Registry 的公开边界只使用 TypeKey、ISystem 引用和 Result；Pipeline / DTO 不应参与其中。
	const Ayin::SystemDescriptor* descriptor =
		Ayin::SystemRegistry::GetSystemDescriptor("SandBox.Tests.SerializationSystem");
	const bool descriptorPassed = descriptor != nullptr &&
		descriptor->RuntimeId == Ayin::GetSystemID<SerializationSystem>() &&
		descriptor->TypeKey == "SandBox.Tests.SerializationSystem" &&
		descriptor->DisplayName == "Serialization System" &&
		descriptor->DefaultSpecification.PhaseMask == Ayin::SystemPhase::None &&
		descriptor->DefaultSpecification.ModeMask == Ayin::SceneMode::None &&
		descriptor->DefaultSpecification.Order == 0 &&
		Ayin::SystemRegistry::GetSystemDescriptor(descriptor->RuntimeId) == descriptor;

	const std::size_t descriptorCount = Ayin::SystemRegistry::GetAllSystemDescriptors().size();
	const bool duplicateRejected =
		!Ayin::SystemRegistry::Register<EarlySystem>("SandBox.Tests.EarlySystem", "Early System", {}, {}, 0) &&
		Ayin::SystemRegistry::GetAllSystemDescriptors().size() == descriptorCount;

	const auto unknownWrite = Ayin::SystemRegistry::SerializeConfiguration(
		SerializationSystem{}, "SandBox.Tests.Unknown");
	Ayin::Scope<Ayin::ISystem> unknownInstance =
		Ayin::SystemRegistry::CreateSystemBy("SandBox.Tests.SerializationSystem");
	const auto unknownRead = unknownInstance == nullptr
		? Ayin::DeserializeSystemConfigurationResult{ .Error{ "System factory returned null" } }
		: Ayin::SystemRegistry::DeserializeConfiguration(
			*unknownInstance, "SandBox.Tests.Unknown", "{}");
	const bool unknownRejected =
		Ayin::SystemRegistry::CreateSystemBy("SandBox.Tests.Unknown") == nullptr &&
		!unknownWrite && !unknownRead;

	SerializationSystem source;
	source.Exposure = 42;
	const auto writeResult = Ayin::SystemRegistry::SerializeConfiguration(
		source, "SandBox.Tests.SerializationSystem");
	Ayin::Scope<Ayin::ISystem> restored =
		Ayin::SystemRegistry::CreateSystemBy(Ayin::GetSystemID<SerializationSystem>());
	const auto readResult = restored == nullptr
		? Ayin::DeserializeSystemConfigurationResult{ .Error{ "System factory returned null" } }
		: Ayin::SystemRegistry::DeserializeConfiguration(
			*restored, Ayin::GetSystemID<SerializationSystem>(), writeResult.Json);
	const SerializationSystem* restoredSystem = restored == nullptr
		? nullptr
		: static_cast<const SerializationSystem*>(restored.get());
	const bool configurationPassed = writeResult && readResult && restoredSystem != nullptr &&
		restoredSystem->Exposure == 42;

	EarlySystem systemWithoutConfiguration;
	const bool emptyConfigurationPassed =
		Ayin::SystemRegistry::DeserializeConfiguration(
			systemWithoutConfiguration, "SandBox.Tests.EarlySystem", "{}") &&
		!Ayin::SystemRegistry::DeserializeConfiguration(
			systemWithoutConfiguration, "SandBox.Tests.EarlySystem", "{\"Unexpected\":1}");

	m_State.RegistryPassed = descriptorPassed && duplicateRejected && unknownRejected &&
		configurationPassed && emptyConfigurationPassed;
	if (!m_State.RegistryPassed) {
		SetFailure("SystemRegistry descriptor, codec or error boundary mismatch");
	}

	return m_State.RegistryPassed;
}

bool SystemTestLayer::CheckEditorInteractionBoundaries() {
	bool passed = true;

	// Preview 由当前 Schedule 序列化而来，但它只用于编辑显示，不应 Attach 或复用实时实例。
	m_State.LifecycleTrace.clear();
	Ayin::SystemPipelineEditor pipelineEditor;
	const bool editorBegan = pipelineEditor.Begin(m_World->GetSystemSchedule());
	Ayin::SystemSchedule& preview = pipelineEditor.GetPreviewSchedule();
	Ayin::ISystem* runtimeSystem = m_World->FindSystemInstance(Ayin::GetSystemID<EarlySystem>());
	Ayin::ISystem* previewSystem = preview.FindSystemInstance(Ayin::GetSystemID<EarlySystem>());
	const bool previewBoundaryPassed = editorBegan &&
		preview.IsBuilt() && !preview.IsAttached() &&
		previewSystem != nullptr && previewSystem != runtimeSystem &&
		m_State.LifecycleTrace.empty();

	const int editorGuiCountBefore = m_State.EditorGuiCount["Early"];
	if (previewSystem != nullptr)
		previewSystem->OnEditorGui();
	const bool previewGuiPassed =
		m_State.EditorGuiCount["Early"] == editorGuiCountBefore + 1 &&
		m_State.LifecycleTrace.empty();
	pipelineEditor.Cancel();
	m_State.EditorInteractionPassed = previewBoundaryPassed && previewGuiPassed;
	if (!m_State.EditorInteractionPassed) {
		SetFailure("SystemPipelineEditor preview boundary mismatch");
		passed = false;
	}

	// Simulation / Runtime 的结构 Apply 必须先结束临时 World，再把新 Pipeline 提交到 EditorWorld。
	m_State.LifecycleTrace.clear();
	Ayin::EditorSession editorSession{ m_Scene, m_Pipeline };
	const bool simulationBegan = editorSession.BeginSimulation();
	Ayin::SystemPipeline::Builder replacementBuilder;
	replacementBuilder.AddSystem<EarlySystem>(
		{ Ayin::SystemPhase::Update },
		{ Ayin::SceneMode::Editor, Ayin::SceneMode::Simulation });
	const Ayin::SystemPipeline replacementPipeline = replacementBuilder.Build();
	m_State.LifecycleTrace.clear();
	const bool sessionApplied = simulationBegan && editorSession.ApplyPipeline(replacementPipeline);
	const std::vector<std::string> expectedSessionApplyTrace{
		"Late:End:Simulation", "Early:End:Simulation",
		"Runtime:Detach", "Late:Detach", "Early:Detach",
		"Runtime:Detach", "Late:Detach", "Early:Detach",
		"Early:Attach"
	};
	const bool temporaryStopped = editorSession.GetTemporaryWorld() == nullptr;
	const bool persistentPipelineUpdated = editorSession.GetPipeline().GetDefinitions().size() == 1 &&
		editorSession.GetEditorWorld().FindSystemInstance(Ayin::GetSystemID<EarlySystem>()) != nullptr &&
		editorSession.GetEditorWorld().FindSystemInstance(Ayin::GetSystemID<LateSystem>()) == nullptr;
	const bool sessionApplyLifecyclePassed =
		m_State.LifecycleTrace == expectedSessionApplyTrace;
	const bool newTemporaryWorldUsesPipeline =
		editorSession.BeginSimulation() &&
		editorSession.GetTemporaryWorld()->FindSystemInstance(Ayin::GetSystemID<EarlySystem>()) != nullptr &&
		editorSession.GetTemporaryWorld()->FindSystemInstance(Ayin::GetSystemID<LateSystem>()) == nullptr;
	editorSession.StopTemporaryWorld();
	m_State.EditorSessionPassed = sessionApplied && temporaryStopped &&
		persistentPipelineUpdated && newTemporaryWorldUsesPipeline &&
		sessionApplyLifecyclePassed;
	if (!m_State.EditorSessionPassed) {
		SetFailure("EditorSession multi-World Apply boundary mismatch");
		passed = false;
	}

	return passed;
}

bool SystemTestLayer::CheckSystemSerialization() {
	// Document 中的 Phases / Modes 是离散枚举数组，而不是底层整数 mask。
	// 这个检查同时覆盖 Document、Glaze metadata，以及 Schedule -> Builder 的映射失败边界。
	Ayin::SystemPipeline::Builder sourceBuilder;
	sourceBuilder.AddSystem(Ayin::SystemDefinition{
		.Type{"SandBox.Tests.SerializationSystem"},
		.Specification{
			.PhaseMask{Ayin::SystemPhase::Update | Ayin::SystemPhase::Presentation},
			.ModeMask{Ayin::SceneMode::Editor | Ayin::SceneMode::Runtime},
			.Order{7}
		}
	});
	Ayin::SystemSchedule source = sourceBuilder.Build().CreateSchedule();
	Ayin::ISystem* sourceInstance = source.FindSystemInstance(Ayin::GetSystemID<SerializationSystem>());
	if (sourceInstance == nullptr) {
		SetFailure("serialization source system was not created");
		return false;
	}

	static_cast<SerializationSystem*>(sourceInstance)->Exposure = 42;
	const std::optional<Ayin::SystemPipelineDocument> document =
		Ayin::SystemScheduleSerializer::Serialize(source);
	if (!document || document->Systems.size() != 1) {
		SetFailure("Schedule did not produce one System Pipeline document entry");
		return false;
	}

	const Ayin::SystemPipelineEntryDocument& entry = document->Systems.front();
	const bool maskShapePassed =
		entry.Phases == std::vector<Ayin::SystemPhase>{ Ayin::SystemPhase::Update, Ayin::SystemPhase::Presentation } &&
		entry.Modes == std::vector<Ayin::SceneMode>{ Ayin::SceneMode::Editor, Ayin::SceneMode::Runtime };

	auto writtenJson = glz::write_json(*document);
	if (!writtenJson) {
		SetFailure("System Pipeline document JSON write failed");
		return false;
	}

	const bool namedMaskPassed =
		writtenJson->find("\"Update\"") != std::string::npos &&
		writtenJson->find("\"Presentation\"") != std::string::npos &&
		writtenJson->find("\"Editor\"") != std::string::npos &&
		writtenJson->find("\"Runtime\"") != std::string::npos;
	m_State.MaskJsonPassed = maskShapePassed && namedMaskPassed;

	const std::optional<Ayin::SystemPipelineDocument> parsedDocument =
		Ayin::SystemScheduleSerializer::Parse(*writtenJson);
	if (!parsedDocument) {
		SetFailure("System Pipeline document JSON read failed");
		return false;
	}

	auto builder = Ayin::SystemScheduleSerializer::Deserialize(*parsedDocument);
	if (!builder) {
		SetFailure("System Pipeline document did not create a Builder");
		return false;
	}

	Ayin::SystemPipeline pipeline = builder->Build();
	Ayin::SystemSchedule restored = pipeline.CreateSchedule();
	const SerializationSystem* restoredSystem = static_cast<const SerializationSystem*>(
		restored.FindSystemInstance(Ayin::GetSystemID<SerializationSystem>()));

	m_State.SerializationRoundTripPassed = m_State.MaskJsonPassed && pipeline.IsValid() &&
		restored.IsBuilt() && restoredSystem != nullptr && restoredSystem->Exposure == 42;
	if (!m_State.SerializationRoundTripPassed) {
		SetFailure("System Pipeline document round-trip did not preserve masks or configuration");
		return false;
	}

	// Parse 只负责 JSON 语法和 DTO；Registry 身份与重复 System 在 Document -> Builder 时拒绝。
	Ayin::SystemPipelineDocument unknownDocument = *parsedDocument;
	unknownDocument.Systems.front().Type = "SandBox.Tests.UnknownSystem";

	Ayin::SystemPipelineDocument duplicateDocument = *parsedDocument;
	duplicateDocument.Systems.emplace_back(duplicateDocument.Systems.front());

	Ayin::SystemPipelineDocument invalidConfigurationDocument = *parsedDocument;
	invalidConfigurationDocument.Systems.front().Configuration = ::glz::raw_json{ "{\"Exposure\":" };
	auto invalidConfigurationBuilder = Ayin::SystemScheduleSerializer::Deserialize(invalidConfigurationDocument);
	const Ayin::SystemPipeline invalidConfigurationPipeline = invalidConfigurationBuilder
		? invalidConfigurationBuilder->Build()
		: Ayin::SystemPipeline{};
	const Ayin::SystemSchedule invalidConfigurationSchedule = invalidConfigurationPipeline.CreateSchedule();

	m_State.SerializationFailurePassed =
		!Ayin::SystemScheduleSerializer::Parse("{\"Systems\":[") &&
		!Ayin::SystemScheduleSerializer::Deserialize(unknownDocument) &&
		!Ayin::SystemScheduleSerializer::Deserialize(duplicateDocument) &&
		invalidConfigurationBuilder && invalidConfigurationPipeline.IsValid() &&
		!invalidConfigurationSchedule.IsBuilt();
	if (!m_State.SerializationFailurePassed)
		SetFailure("System Pipeline document failure boundary mismatch");

	return m_State.SerializationRoundTripPassed && m_State.SerializationFailurePassed;
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
	RenderCheck("Direct System editor GUI", m_State.EditorGuiPassed);
	RenderCheck("Read-only System observation", m_State.SystemObservationPassed);
	RenderCheck("World lifecycle", m_State.WorldLifecyclePassed);
	RenderCheck("Duplicate add", m_State.DuplicateAddPassed);
	RenderCheck("Schedule structure frozen", m_State.RemovePassed);
	RenderCheck("Attach registration order", m_State.AttachOrderPassed);
	RenderCheck("Begin/End reverse order", m_State.BeginEndOrderPassed);
	RenderCheck("Mode transition", m_State.TransitionPassed);
	RenderCheck("Destructor fallback cleanup", m_State.DestructorCleanupPassed);
	RenderCheck("Schedule move construction", m_State.MoveConstructionPassed);
	RenderCheck("Schedule legacy baseline", m_State.ScheduleBaselinePassed);
	RenderCheck("Pipeline Builder model", m_State.BuilderModelPassed);
	RenderCheck("System Registry", m_State.RegistryPassed);
	RenderCheck("Mask JSON", m_State.MaskJsonPassed);
	RenderCheck("Serialization round-trip", m_State.SerializationRoundTripPassed);
	RenderCheck("Serialization failure boundary", m_State.SerializationFailurePassed);
	RenderCheck("Live World frame", m_State.LiveFramePassed);
	RenderCheck("World Apply", m_State.ApplyPassed);
	RenderCheck("World Apply failure boundary", m_State.ApplyFailurePassed);
	RenderCheck("Editor interaction boundaries", m_State.EditorInteractionPassed);
	RenderCheck("EditorSession Apply", m_State.EditorSessionPassed);
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
		++s_State->BeginCount[Name()];
		s_State->LifecycleTrace.emplace_back(
			std::string{ Name() } + ":Begin:" + ModeName(context.Mode));
	}
}

void SystemTestLayer::ProbeSystem::OnEditorGui() {
	if (s_State != nullptr)
		++s_State->EditorGuiCount[Name()];
}

void SystemTestLayer::ProbeSystem::OnEnd(const Ayin::SystemContext& context) {
	if (s_State != nullptr) {
		if (&context.Scene != s_State->ExpectedScene || context.Phase != Ayin::SystemPhase::None) {
			s_State->ContextValid = false;
		}
		++s_State->EndCount[Name()];
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