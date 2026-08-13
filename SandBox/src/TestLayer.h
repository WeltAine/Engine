#pragma once

#include "Ayin/Core/Layer.h"
#include "Ayin/Core/Timestep.h"
#include "Ayin/Scene/Scene.h"
#include "Ayin/System/SystemPipeline.h"
#include "Ayin/System/SystemSchedule.h"
#include "Ayin/System/World.h"

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

//! 这是 SandBox 中的系统调度测试入口。
//! 测试顺序大致是：构建 Pipeline -> 创建 World -> 检查生命周期和调度 -> 检查真实应用帧。
// SandBox 是一次性的测试宿主，因此这里把 SystemSchedule、SystemPipeline
// 和 World 的行为测试集中放在一个 Layer 中，避免为暂时的测试单独建立目标。
class TestLayer final : public Ayin::Layer {
public:
	TestLayer();
	~TestLayer() override;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Ayin::Timestep deltaTime) override;
	void OnImGuiRender() override;

private:
	struct TestState final {
		// 用映射按 System 名称分别统计生命周期回调，避免多个测试 System 的计数混在一起。
		std::unordered_map<std::string, int> AttachCount;
		std::unordered_map<std::string, int> DetachCount;
		// Trace 只保存最近一次调度的执行轨迹，例如 Early:Update。
		std::vector<std::string> Trace;
		std::string Failure;
		const Ayin::Scene* ExpectedScene = nullptr;
		float ExpectedDelta = 0.0f;

		// Pipeline 和各种行为检查的结果，最后统一显示在 ImGui 面板中。
		bool PipelineBuilt = false;
		bool AutomaticOrderPassed = false;
		bool ExplicitOrderPassed = false;
		bool ModeFilteringPassed = false;
		bool ContextForwardingPassed = false;
		bool DuplicateAddPassed = false;
		bool RemovePassed = false;
		bool WorldLifecyclePassed = false;
		bool LiveFramePassed = false;
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

	class LifecycleSystem final : public ProbeSystem {
	protected:
		// 单独用于测试重复添加、移除和 OnDetach 生命周期。
		const char* Name() const override { return "Lifecycle"; }
	};

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
