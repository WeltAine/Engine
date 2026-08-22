#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/System/SystemContext.h"
#include "Ayin/System/SystemTypes.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

namespace Ayin {

	// 系统条目只保存运行时实例及其观察信息。
	// 阶段执行关系由 Schedule 的索引计划维护，避免把所有权之外的裸指针长期保存下来。
	struct SystemEntry {

		friend class SystemSchedule;

		SystemInformation Information;

		SystemSpecification Specification;

		SystemEntry() = default;
		SystemEntry(SystemInformation information, SystemSpecification specification, Scope<ISystem> instance)
			: Information{ std::move(information) }, Specification{ specification }, Instance{ std::move(instance) } {};

		ISystem* GetInstance() { return Instance.get(); };
		const ISystem* GetInstance() const { return Instance.get(); };


	private:

		Scope<ISystem> Instance;


	};

	// Editor 使用的只读运行时视图；View 不拥有 System，也不能替换 Schedule 拓扑。
	//? 既然是编辑器中使用，和 Ayin 核心无关，那是否应该出现在这里？还是应该放在 Ayin-Editor 模块中？关于防止Order 被更改或者 可用模式被修改，是否应该让 Ayin-Editor 来承担，而非 Ayin-Core？毕竟 Ayin-Core 只是提供了一个 Schedule 的运行时视图，至于编辑器中如何使用这个视图，应该由 Ayin-Editor 来决定。
	struct SystemRuntimeView {

		const SystemInformation* Information = nullptr;
		const SystemSpecification* Specification = nullptr;
		const ISystem* Instance = nullptr;

	};


	class SystemSchedule {

		friend class SystemPipeline;
		friend class World;

	public:

		using SystemIndex = std::size_t;

		SystemSchedule() = default;
		~SystemSchedule();
		SystemSchedule(const SystemSchedule&) = delete;
		SystemSchedule& operator=(const SystemSchedule&) = delete;
		SystemSchedule(SystemSchedule&& other) noexcept;
		SystemSchedule& operator=(SystemSchedule&& other) noexcept;


		void Begin(const SystemContext& systemContext);
		void Run(const SystemContext& context);
		void End(const SystemContext& systemContext);
		// 显式结束当前会话并释放所有 System。调用者必须提供真实上下文。
		void Shutdown(const SystemContext& systemContext);

		inline bool IsActive() const { return m_LifecycleState == LifecycleState::Active; };
		inline bool IsBuilt() const { return m_TopologySealed; };
		inline bool IsAttached() const { return m_Attached; };


		const std::vector<SystemEntry>& GetSystems() const { return m_Systems; };
		std::vector<SystemRuntimeView> GetRuntimeViews() const;

		ISystem* FindSystemInstance(SystemID systemId);
		const ISystem* FindSystemInstance(SystemID systemId) const;
		ISystem* FindSystemInstance(std::string_view systemName);
		const ISystem* FindSystemInstance(std::string_view systemName) const;

		const SystemEntry* FindSystemEntry(SystemID systemId) const;
		const SystemEntry* FindSystemEntry(std::string_view systemName) const;


	private:

		enum class LifecycleState : uint8_t {

			Idle,
			Active

		};

		static constexpr std::size_t PhaseCount = 4;

		// 这些构建过程只有友元类可以碰，保证安全性
		bool BeginConstruction();	// 开始构建拓扑关系，清空所有系统，标记 m_TopologySealed = false
		bool BuildSystem(const SystemDefinition& definition);	// 构建单个系统，创建实例并保存到 m_Systems 中
		bool FinishConstruction();	// 构建完成后（完成各阶段内部系统顺序规划），拓扑关系被封印，不能再修改

		//! 重在调用 各系统的 OnAttach() / OnDetach()，设置 m_AttachSequence，标记 m_Attached，不会改变拓扑结构
		bool AttachSystems();		// 根据 m_Systems 顺序触发 OnAttach()，并记录顺序到 m_AttachSequence，标记 m_Attached = true
		void DetachSystems();		// 根据 m_AttachSequence 顺序触发 OnDetach()，清空 m_AttachSequence，标记 m_Attached = false
		//? 我觉得 m_AttachSequence 应该在构建完成时也确定下来，而不是 AttachSystem() 的时候才决定，太怪了，也迁移到 RebuildExecutionPlans 中吧

		void ClearSystems();		// 清空所有系统（可以说就是 Schedule 的 Clear 键），调用 DetachSystems()，清空 m_Systems，标记 m_TopologySealed = false	（只有该方法会解开拓扑封印）

		std::vector<SystemIndex>& GetPhasePlan(SystemPhase phase);
		const std::vector<SystemIndex>& GetPhasePlan(SystemPhase phase) const;

		void RebuildExecutionPlans();
		void MoveFrom(SystemSchedule&& other) noexcept;

		std::vector<SystemEntry>::iterator FindSystem(SystemID systemId);
		std::vector<SystemEntry>::const_iterator FindSystem(SystemID systemId) const;


	private:

		std::vector<SystemEntry> m_Systems;

		// Attach 顺序、Begin 顺序和各 Update 阶段都保存独立索引计划。
		std::vector<SystemIndex> m_AttachSequence;		// 在 Atach 的时候记录顺序，而非根据 m_attachSequence 决定顺序
		std::vector<SystemIndex> m_BeginPlan;
		std::array<std::vector<SystemIndex>, PhaseCount> m_PhasePlans;
		std::vector<SystemIndex> m_BegunSystems;

		LifecycleState m_LifecycleState = LifecycleState::Idle;
		bool m_TopologySealed = false;										// schedule 是否已经构建完成，拓扑关系被封印（即不可改动）
		bool m_Attached = false;

	};


};