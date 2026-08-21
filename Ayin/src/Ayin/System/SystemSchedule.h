#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/System/SystemContext.h"
#include "Ayin/System/SystemTypes.h"
#include "Ayin/System/SystemRegistry.h"

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

//! 暂时只是 Scene 存储各个系统，没有一个更上层的东西，要用这个文件还需要解决系统生命周期管理的问题，用 Ref ？或者是单例？


namespace Ayin {

	// 阶段条目
	struct PhaseSystemEntry : ISystem {

		SystemID RuntimeId;

		int Order = 0;

		SceneMode ModeMask = SceneMode::None;

		ISystem* raw_ptr = nullptr;								//! 观测式系统实例



		virtual inline void OnPreUpdate(const SystemContext& systemContext) { if (raw_ptr != nullptr && static_cast<bool>(ModeMask & systemContext.Mode)) raw_ptr->OnPreUpdate(systemContext); };
		virtual inline void OnUpdate(const SystemContext& systemContext) { if (raw_ptr != nullptr && static_cast<bool>(ModeMask & systemContext.Mode)) raw_ptr->OnUpdate(systemContext); };
		virtual inline void OnPostUpdate(const SystemContext& systemContext) { if (raw_ptr != nullptr && static_cast<bool>(ModeMask & systemContext.Mode)) raw_ptr->OnPostUpdate(systemContext); };
		virtual inline void OnPresentationUpdate(const SystemContext& systemContext) { if (raw_ptr != nullptr && static_cast<bool>(ModeMask & systemContext.Mode)) raw_ptr->OnPresentationUpdate(systemContext); };


		// --------------------------- Compare ------------------------------

		struct PhaseSystemEntryCompareByOrder {

			inline bool operator() (const PhaseSystemEntry& leftEntry, const PhaseSystemEntry& rightEntry) const {

				if (leftEntry.RuntimeId != rightEntry.RuntimeId && leftEntry.Order != rightEntry.Order)
					return leftEntry.Order < rightEntry.Order;

				return leftEntry.RuntimeId < rightEntry.RuntimeId;

			};

		};


	};

	// 系统条目
	struct SystemEntry {

		SystemInformation Information;

		SystemSpecification Specification;

		Scope<ISystem> Instance;

		inline operator PhaseSystemEntry() const {

			PhaseSystemEntry entry{};

			entry.RuntimeId = Information.RuntimeId;
			entry.Order = Specification.Order;
			entry.ModeMask = Specification.ModeMask;

			entry.raw_ptr = Instance.get();

			return entry;

		};


		// ------------------------------ Compare --------------------------------

		struct SystemEntryCompareByOrder {

			//? 比较器为什么一定是 const
			//! std::set 把比较器当作容器规则，而不是可变状态。它会通过一个 const Compare 对象调用： operator()
			//! 末尾的 const 表示比较过程不修改比较器自身。否则标准库无法在 const 上下文中安全使用它。
			inline bool operator() (const SystemEntry& leftEntry, const SystemEntry& rightEntry) const {

				if (leftEntry.Information.RuntimeId != rightEntry.Information.RuntimeId && leftEntry.Specification.Order != rightEntry.Specification.Order)
					return leftEntry.Specification.Order < rightEntry.Specification.Order;

				return leftEntry.Information.RuntimeId < rightEntry.Information.RuntimeId;

			};

		};


	};
	

	// 本质是 Schedule 的阶段筛，一种存储结构
	class SchedulePhase {

	public:

		~SchedulePhase() = default;

		SchedulePhase& AddSystem(const PhaseSystemEntry& phaseSystemEntry);

		template<typename System>
			requires std::derived_from<System, ISystem>
		inline SchedulePhase& RemoveSystem() {

			SystemID id = GetSystemID<System>();

			auto it = std::ranges::find_if(m_OrderedSystems,
				[&id](const PhaseSystemEntry& entry) -> bool {return entry.RuntimeId == id; }
			);

			if (it != m_OrderedSystems.end())
				m_OrderedSystems.erase(it);

			return *this;

		};

		SchedulePhase& RemoveSystem(const SystemID systemId);

		inline std::set<PhaseSystemEntry, PhaseSystemEntry::PhaseSystemEntryCompareByOrder>::iterator begin() { return m_OrderedSystems.begin(); };
		inline std::set<PhaseSystemEntry, PhaseSystemEntry::PhaseSystemEntryCompareByOrder>::iterator end() { return m_OrderedSystems.end(); };
		inline std::set<PhaseSystemEntry, PhaseSystemEntry::PhaseSystemEntryCompareByOrder>::const_iterator begin() const { return m_OrderedSystems.begin(); };
		inline std::set<PhaseSystemEntry, PhaseSystemEntry::PhaseSystemEntryCompareByOrder>::const_iterator end() const { return m_OrderedSystems.end(); };

		inline std::set<PhaseSystemEntry, PhaseSystemEntry::PhaseSystemEntryCompareByOrder>::reverse_iterator rbegin() { return m_OrderedSystems.rbegin(); };
		inline std::set<PhaseSystemEntry, PhaseSystemEntry::PhaseSystemEntryCompareByOrder>::reverse_iterator rend() { return m_OrderedSystems.rend(); };
		inline std::set<PhaseSystemEntry, PhaseSystemEntry::PhaseSystemEntryCompareByOrder>::const_reverse_iterator rbegin() const { return m_OrderedSystems.rbegin(); };
		inline std::set<PhaseSystemEntry, PhaseSystemEntry::PhaseSystemEntryCompareByOrder>::const_reverse_iterator rend() const { return m_OrderedSystems.rend(); };

	private:
		std::set<PhaseSystemEntry, PhaseSystemEntry::PhaseSystemEntryCompareByOrder> m_OrderedSystems;		//前一个是集合类型，后一个提供比较函数
		//? 为什么 Set 会提供如此奇怪的设计？ 比较器类型和元素类型分离？常规直觉不应该是元素类型提供比较函数，没有提供的话使用默认比较方法？
		//! “数据是什么”和“如何排序”是两个不同概念。同一种元素可能按 ID、名称或执行顺序建立不同的集合： std::set<System, CompareById> 和 std::set<System, CompareByOrder>

	};


	// -----------------------------------------------------------------------------------------------------------------------------



	class SystemSchedule {

	public:

		SystemSchedule() = default;
		~SystemSchedule();
		SystemSchedule(const SystemSchedule&) = delete;
		SystemSchedule& operator=(const SystemSchedule&) = delete;	//! 会有旧状态的处理，可以有，但暂时没什么不要，需要新的 Schedule 建议直接 Pipeline 构建
		SystemSchedule(SystemSchedule&&) noexcept = default;
		SystemSchedule& operator=(SystemSchedule&&) noexcept;		//! 为 World 更新 系统调度 做准备


		void Begin(const SystemContext& systemContext);
        void Run(const SystemContext& context);
		void End(const SystemContext& systemContext);



		SystemSchedule& AddSystem(const SystemDefinition& definition);


        template<typename System>
            requires std::derived_from<System, ISystem>&& std::default_initializable<System>
		inline SystemSchedule& AddSystem(const std::vector<SystemPhase>& phases, const std::vector<SceneMode>& modes, int order);

		template<typename System>
			requires std::derived_from<System, ISystem>&& std::default_initializable<System>
		inline SystemSchedule& AddSystem(const std::vector<SystemPhase>& phases, const std::vector<SceneMode>& modes);


		template<typename System>
			requires std::derived_from<System, ISystem>&& std::default_initializable<System>
		inline SystemSchedule& RemoveSystem();


		void Clear();
		SystemEntry* FindSystemEntry(SystemID systemId);
		const SystemEntry* FindSystemEntry(SystemID systemId) const;
		SystemEntry* FindSystemEntry(std::string_view systemName);
		const SystemEntry* FindSystemEntry(std::string_view systemName) const;
		const std::vector<SystemEntry>& GetSystems() const { return m_Systems; };
		std::vector<SystemEntry>& GetSystems() { return m_Systems; };


		template<typename System>
			requires std::derived_from<System, ISystem>&& std::default_initializable<System>
		inline bool Contain() const { return FindSystem<System>() != m_Systems.end(); };

		inline bool Contain(SystemID systemId) const { return FindSystem(systemId) != m_Systems.end(); };

	private:

		template<typename System>
			requires std::derived_from<System, ISystem>
		inline std::vector<SystemEntry>::iterator FindSystem();

		template<typename System>
			requires std::derived_from<System, ISystem>
		inline std::vector<SystemEntry>::const_iterator FindSystem() const;

		std::vector<SystemEntry>::iterator FindSystem(SystemID systemId);

		std::vector<SystemEntry>::const_iterator FindSystem(SystemID systemId) const;


	private:

		void InsertSystemToPhase(const PhaseSystemEntry& phaseSystemEntry, const std::vector<SystemPhase>& phases);

	private:

		std::vector<SystemEntry> m_Systems;
		std::vector<SystemID> m_BegunSystems;	// 执行过 OnBegin 的 System

		int m_NextOrder = 0;

		SchedulePhase m_PreUpdate_Phase;
		SchedulePhase m_Update_Phase;
		SchedulePhase m_PostUpdate_Phase;
		SchedulePhase m_Presentation_Phase;

	};


	template<typename System>
		requires std::derived_from<System, ISystem>&& std::default_initializable<System>
	inline SystemSchedule& SystemSchedule::AddSystem(const std::vector<SystemPhase>& phases, const std::vector<SceneMode>& modes, int order) {

		auto it = FindSystem<System>();
		if (it != m_Systems.end())
			return *this;

		SystemPhase phaseMask = SystemPhase::None;
		for (const SystemPhase phase : phases)
			phaseMask |= phase;

		SceneMode modeMask = SceneMode::None;
		for (const SceneMode mode : modes)
			modeMask |= mode;

		const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(GetSystemID<System>());
		if (descriptor == nullptr) {
			AYIN_CORE_ERROR("System RuntimeId is not registered");
			return *this;
		}

		//！触发移动语义
		m_Systems.emplace_back(
			std::move<SystemEntry>(
				SystemEntry{
				.Information{.RuntimeId{descriptor->RuntimeId}, .TypeKey{descriptor->TypeKey}},
				.Specification{.PhaseMask{phaseMask}, .ModeMask{modeMask}, .Order{order}},
				.Instance{CreateScope<System>()},
				}
				));
		m_Systems.back().Instance->OnAttach();
		m_NextOrder = std::max(m_NextOrder, order + 1);

		PhaseSystemEntry phaseEntry = (PhaseSystemEntry)(m_Systems.back());

		InsertSystemToPhase(phaseEntry, phases);

		return *this;
	}

	template<typename System>
		requires std::derived_from<System, ISystem>&& std::default_initializable<System>
	inline SystemSchedule& SystemSchedule::AddSystem(const std::vector<SystemPhase>& phases, const std::vector<SceneMode>& modes) {

		return AddSystem<System>(phases, modes, m_NextOrder);

	}

	template<typename System>
		requires std::derived_from<System, ISystem>&& std::default_initializable<System>
	inline SystemSchedule& SystemSchedule::RemoveSystem() {

		m_PreUpdate_Phase.RemoveSystem<System>();
		m_Update_Phase.RemoveSystem<System>();
		m_PostUpdate_Phase.RemoveSystem<System>();
		m_Presentation_Phase.RemoveSystem<System>();


		auto it = FindSystem<System>();
		if (it != m_Systems.end()) {
			it->Instance->OnDetach();
			m_Systems.erase(it);

		}

		return *this;

	};


	template<typename System>
		requires std::derived_from<System, ISystem>
	inline std::vector<SystemEntry>::iterator SystemSchedule::FindSystem() {

		auto it = std::ranges::find_if(
			m_Systems,
			[](const SystemEntry& entry) ->bool {
				return entry.Information.RuntimeId == GetSystemID<System>();
			}
		);

		return it;
	};

	template<typename System>
		requires std::derived_from<System, ISystem>
	inline std::vector<SystemEntry>::const_iterator SystemSchedule::FindSystem() const {

		auto it = std::ranges::find_if(
			m_Systems,
			[](const SystemEntry& entry)->bool {
				return entry.Information.RuntimeId == GetSystemID<System>();
			}
		);

		return it;
	};


};
