#pragma once

#include "Ayin/Core/Layer.h"
#include "Ayin/Core/Timestep.h"
#include "Ayin/Scene/Scene.h"
#include "Ayin/System/SystemPipeline.h"
#include "Ayin/System/SystemPipelineEditor.h"
#include "Ayin/System/EditorSession.h"
#include "Ayin/System/SystemRegistry.h"
#include "Ayin/System/SystemSchedule.h"
#include "Ayin/System/SystemScheduleSerializer.h"
#include "Ayin/System/World.h"

#include <array>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

//! 独立的 System/World 回归测试 Layer。
//!
//! 测试流程总览（每一个箭头都对应本类中的一个实际调用阶段）：
//!
//!  0. SandBoxApp 读取 AYIN_SANDBOX_TEST_LAYER；只有值为 "system" 时才创建本 Layer。
//!     AYIN_SANDBOX_TEST_AUTO_EXIT 为真时，测试结束后会自动关闭 SandBox，供脚本/CI 使用。
//!
//!  1. OnAttach()
//!       -> ProbeSystem::Bind(m_State)
//!          将所有测试 System 的静态观测入口绑定到本 Layer 的 TestState。
//!       -> BuildWorld()
//!          注册全部测试 System，构建 Pipeline、Scene 与 World，
//!          并立刻验证 OnAttach 是否按注册顺序发生。
//!       -> RunOneShotChecks()
//!          不依赖真实应用帧，直接执行下面三组确定性规则检查：
//!          a) CheckPipelineAndWorld()：World 的非法状态、模式过滤、Begin/End 正逆序、
//!             TransitionMode、SystemContext 转发、空 Scene 拒绝。
//!          b) CheckScheduleLifecycle()：裸 SystemSchedule 的重复注册、显式 Order、
//!             Update 阶段执行和单次生命周期调用。
//!          c) CheckDestructorCleanupAndMove()：Schedule / World 析构兜底清理，以及
//!             SystemSchedule 移动构造后不会遗漏或重复生命周期回调。
//!          d) CheckScheduleBaseline()：验证裸 Schedule 的状态机规则：Begin 前 Run 拒绝、
//!             重复 Begin / End 无操作以及 World 控制的生命周期边界。
//!          e) CheckSystemRegistry()：验证稳定 TypeKey、描述符、工厂、配置 Codec 和失败结果。
//!          f) CheckSystemSerialization()：验证 mask 的 JSON 表达，以及现有 DTO 到
//!             Builder 的 round-trip 骨架。
//!          g) CheckEditorInteractionBoundaries()：验证 Preview 与实时实例隔离，以及
//!             Simulation Apply 回到 EditorWorld 的多 World 语义。
//!
//!  2. OnUpdate(deltaTime)
//!       -> 若一次性检查全部通过，RunLiveFrame(deltaTime) 使用 Application 提供的真实帧时间
//!          执行 BeginWorldExecutionSession(Editor) -> Update -> EndWorldExecutionSession，
//!          验证最终主循环路径，而不仅是直接调用 Schedule 的单元式路径。
//!
//!  3. 结果处理
//!       -> 任一断言失败：SetFailure() 保留第一条原因并记录 SYSTEM_WORLD_TEST: FAIL。
//!       -> 全部通过：记录 SYSTEM_WORLD_TEST: PASS，并在 ImGui 面板显示各项检查结果与轨迹。
//!
//!  4. OnDetach()
//!       -> 销毁 World，使其与内部 Schedule 执行最后的析构兜底；随后解除 ProbeSystem 绑定。
//!
//! Trace 记录 Update 四阶段的实际调用顺序；LifecycleTrace 记录
//! Attach / Begin / End / Detach，因此测试不仅断言“是否调用”，还断言“调用顺序是否正确”。
// 与其他模块测试隔离；由 SandBoxApp 根据环境变量决定是否创建该 Layer。
class SystemTestLayer final : public Ayin::Layer {
public:
	SystemTestLayer();
	~SystemTestLayer() override;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Ayin::Timestep deltaTime) override;
	void OnImGuiRender() override;

private:
	struct TestState final {
		// 用映射按 System 名称分别统计生命周期回调，避免多个测试 System 的计数混在一起。
		std::unordered_map<std::string, int> AttachCount;
		std::unordered_map<std::string, int> DetachCount;
		std::unordered_map<std::string, int> BeginCount;
		std::unordered_map<std::string, int> EndCount;
		std::unordered_map<std::string, int> EditorGuiCount;
		// Trace 只保存最近一次调度的执行轨迹，例如 Early:Update。
		std::vector<std::string> Trace;
		// 生命周期轨迹用于验证正序进入和逆序退出规则。
		std::vector<std::string> LifecycleTrace;
		std::string Failure;
		const Ayin::Scene* ExpectedScene = nullptr;
		float ExpectedDelta = 0.0f;

		// Pipeline 和各种行为检查的结果，最后统一显示在 ImGui 面板中。
		bool PipelineBuilt = false;
		bool AutomaticOrderPassed = false;
		bool ExplicitOrderPassed = false;
		bool ModeFilteringPassed = false;
		bool ContextForwardingPassed = false;
		bool EditorGuiPassed = false;
		bool DuplicateAddPassed = false;
		bool RemovePassed = false;
		bool AttachOrderPassed = false;
		bool BeginEndOrderPassed = false;
		bool TransitionPassed = false;
		bool DestructorCleanupPassed = false;
		bool MoveConstructionPassed = false;
		bool ScheduleBaselinePassed = false;
		bool BuilderModelPassed = false;
		bool RegistryPassed = false;
		bool MaskJsonPassed = false;
		bool SerializationRoundTripPassed = false;
		bool WorldLifecyclePassed = false;
		bool LiveFramePassed = false;
		bool ApplyPassed = false;
		bool ApplyFailurePassed = false;
		bool EditorInteractionPassed = false;
		bool EditorSessionPassed = false;
		bool ContextValid = true;
		bool Completed = false;
		bool Pass = false;
	};

	// 所有测试 System 共用同一份观测状态，但每个具体类型仍然是独立的 System。
	// 这样既能观察多个 System 的执行顺序，也能验证调度器的生命周期管理。
	class ProbeSystem : public Ayin::ISystem {
	public:
		static void Bind(TestState* state);
		static void Unbind();

		void OnAttach() override;
		void OnDetach() override;
		void OnBegin(const Ayin::SystemContext& context) override;
		void OnEnd(const Ayin::SystemContext& context) override;
		void OnEditorGui() override;

	protected:
		virtual const char* Name() const = 0;
		// 统一记录阶段、模式、场景和 DeltaTime，具体测试 System 不需要重复实现四个回调。
		void RecordPhase(const Ayin::SystemContext& context, const char* phase);
		void OnPreUpdate(const Ayin::SystemContext& context) override { RecordPhase(context, "PreUpdate"); }
		void OnUpdate(const Ayin::SystemContext& context) override { RecordPhase(context, "Update"); }
		void OnPostUpdate(const Ayin::SystemContext& context) override { RecordPhase(context, "PostUpdate"); }
		void OnPresentationUpdate(const Ayin::SystemContext& context) override { RecordPhase(context, "Presentation"); }

		static TestState* s_State;
	};

	class EarlySystem final : public ProbeSystem {
	protected:
		// 自动注册时先加入的 System，期望在 LateSystem 之前执行。
		const char* Name() const override { return "Early"; }
	};

	class LateSystem final : public ProbeSystem {
	protected:
		// 自动注册时后加入的 System，也用于显式顺序测试。
		const char* Name() const override { return "Late"; }
	};

	class RuntimeSystem final : public ProbeSystem {
	protected:
		// 只允许 Runtime 模式执行，用来检查模式掩码过滤。
		const char* Name() const override { return "Runtime"; }
	};

	class FailingAttachSystem final : public Ayin::ISystem {
	public:
		void OnAttach() override {
			throw std::runtime_error{ "intentional attach failure" };
		}
	};

	class LifecycleSystem final : public ProbeSystem {
	protected:
		// 单独用于测试重复添加、移除和 OnDetach 生命周期。
		const char* Name() const override { return "Lifecycle"; }
	};

	// 这个 System 保存一个可序列化的普通字段，用于建立当前序列化链路的 round-trip 基线。
	class SerializationSystem final : public ProbeSystem {
	public:
		int Exposure = 1;

		struct glaze {
			using T = SerializationSystem;
			static constexpr auto value = glz::object(
				"Exposure", &T::Exposure
			);
		};

	protected:
		const char* Name() const override { return "Serialization"; }
	};

	// 注册只在本测试进程内执行一次，避免测试所依赖的全局 Registry 产生重复描述符。
	void RegisterTestSystems();
	// 第一轮检查在 OnAttach 中完成，不依赖应用主循环。
	void RunOneShotChecks();
	// 第二轮检查通过真实 OnUpdate 调用 World，验证应用循环中的实际路径。
	void RunLiveFrame(Ayin::Timestep deltaTime);
	// 构造测试用 Pipeline、Scene 和 World。
	void BuildWorld();
	// 记录第一条失败信息，并输出一次失败日志。
	void SetFailure(const char* failure);
	// 渲染单项测试的结果。
	void RenderCheck(const char* label, bool passed) const;

	// 分别测试 Pipeline/World 和裸 SystemSchedule 的行为。
	bool CheckPipelineAndWorld();
	bool CheckScheduleLifecycle();
	bool CheckDestructorCleanupAndMove();
	bool CheckScheduleBaseline();
	bool CheckPipelineBuilder();
	bool CheckSystemRegistry();
	bool CheckSystemSerialization();
	bool CheckEditorInteractionBoundaries();

	// 比较 SystemContext 中的场景和时间步是否被正确转发。
	std::vector<std::string> ExpectedEditorTrace() const;
	// Runtime 模式下只允许 RuntimeSystem 出现在轨迹中。
	std::vector<std::string> ExpectedRuntimeTrace() const;
	// 将当前状态转换为 ImGui 和日志使用的结果文本。
	const char* ResultText() const;

private:
	Ayin::Ref<Ayin::Scene> m_Scene;
	// Pipeline 负责保存 System 注册信息，World 根据它创建自己的 Schedule。
	Ayin::SystemPipeline m_Pipeline;
	std::unique_ptr<Ayin::World> m_World;
	TestState m_State;

	// 设置后，真实帧测试通过时自动关闭应用，方便命令行验证。
	bool m_AutoExitEnabled = false;
	bool m_AutoExitRequested = false;
	// 防止一次性测试和最终结果被重复执行或重复记录。
	bool m_RanChecks = false;
	bool m_LoggedResult = false;
};
