#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/Scene.h"

#include "Ayin/System/Systems.h"

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <set>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

//! 暂时只是 Scene 存储各个系统，没有一个更上层的东西，要用这个文件还需要解决系统生命周期管理的问题，用 Ref ？或者是单例？


namespace Ayin {

	enum class SystemPhase : uint8_t {

		None = 0,

		PreUpdate = BIT(0),
		Update = BIT(1),
		PostUpdate = BIT(2),
		Presentation = BIT(3)

	};



	constexpr SystemPhase operator| (SystemPhase leftPhase, SystemPhase rightPhase) {
		using UnderlyingType = std::underlying_type_t<SystemPhase>;	// 得到某个枚举实际使用的整数存储类型
		return static_cast<SystemPhase>(static_cast<UnderlyingType>(leftPhase) | static_cast<UnderlyingType>(rightPhase));
	};

	constexpr SystemPhase operator& (SystemPhase leftPhase, SystemPhase rightPhase) {
		using UnderlyingType = std::underlying_type_t<SystemPhase>;	// 得到某个枚举实际使用的整数存储类型
		return static_cast<SystemPhase>(static_cast<UnderlyingType>(leftPhase) & static_cast<UnderlyingType>(rightPhase));
	};

	constexpr SystemPhase operator^ (SystemPhase leftPhase, SystemPhase rightPhase) {
		using UnderlyingType = std::underlying_type_t<SystemPhase>;	// 得到某个枚举实际使用的整数存储类型
		return static_cast<SystemPhase>(static_cast<UnderlyingType>(leftPhase) ^ static_cast<UnderlyingType>(rightPhase));
	};

	constexpr SystemPhase operator~ (SystemPhase mode) {
		using UnderlyingType = std::underlying_type_t<SystemPhase>;	// 得到某个枚举实际使用的整数存储类型
		return static_cast<SystemPhase>(~static_cast<UnderlyingType>(mode));
	};

	constexpr SystemPhase& operator|= (SystemPhase& leftPhase, SystemPhase rightPhase) {
		using UnderlyingType = std::underlying_type_t<SystemPhase>;	// 得到某个枚举实际使用的整数存储类型
		leftPhase = leftPhase | rightPhase;
		return leftPhase;
	};

	constexpr SystemPhase& operator&= (SystemPhase& leftPhase, SystemPhase rightPhase) {
		using UnderlyingType = std::underlying_type_t<SystemPhase>;	// 得到某个枚举实际使用的整数存储类型
		leftPhase = leftPhase & rightPhase;
		return leftPhase;
		//! static_cast 当转换为非引用类型时会产生拷贝操作，也就是返回一个新值
	};

	struct SystemContext {

		Scene& Scene;									//当前更新的场景

		Timestep DeltaTime = {};						//时间间隔

		SceneMode Mode = SceneMode::None;				//场景运行模式

		SystemPhase Phase = SystemPhase::None;			//当前阶段

	};



	// ------------------------------------------------------------------------------------------------------------------------

	struct SystemInformation {

		SystemID Id;
		std::string Name;

	};

	struct SystemSpecification {

		SystemPhase PhaseMask = SystemPhase::None;
		SceneMode ModeMask = SceneMode::None;
		int Order = 0;

	};

	struct PhaseSystemEntry : ISystem {

		SystemID Id;

		int Order = 0;

		SceneMode ModeMask = SceneMode::None;

		ISystem* raw_ptr = nullptr;		//! 观测式系统实例



		virtual inline void OnPreUpdate(const SystemContext& systemContext) { if (raw_ptr != nullptr && static_cast<bool>(ModeMask & systemContext.Mode)) raw_ptr->OnPreUpdate(systemContext); };
		virtual inline void OnUpdate(const SystemContext& systemContext) { if (raw_ptr != nullptr && static_cast<bool>(ModeMask & systemContext.Mode)) raw_ptr->OnUpdate(systemContext); };
		virtual inline void OnPostUpdate(const SystemContext& systemContext) { if (raw_ptr != nullptr && static_cast<bool>(ModeMask & systemContext.Mode)) raw_ptr->OnPostUpdate(systemContext); };
		virtual inline void OnPresentationUpdate(const SystemContext& systemContext) { if (raw_ptr != nullptr && static_cast<bool>(ModeMask & systemContext.Mode)) raw_ptr->OnPresentationUpdate(systemContext); };


		// --------------------------- Compare ------------------------------

		struct PhaseSystemEntryCompareByOrder {

			inline bool operator() (const PhaseSystemEntry& leftEntry, const PhaseSystemEntry& rightEntry) const {

				if (leftEntry.Id != rightEntry.Id && leftEntry.Order != rightEntry.Order)
					return leftEntry.Order < rightEntry.Order;

				return leftEntry.Id < rightEntry.Id;

			};

		};


	};


	struct SystemEntry {

		SystemInformation Information;

		SystemSpecification Specification;

		Scope<ISystem> Instance;

		inline operator PhaseSystemEntry() const {

			PhaseSystemEntry entry{};

			entry.Id = Information.Id;
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
			inline bool operator() (const SystemEntry& leftEntry, const SystemEntry& rightEntry) {

				if (leftEntry.Information.Id != rightEntry.Information.Id && leftEntry.Specification.Order != rightEntry.Specification.Order)
					return leftEntry.Specification.Order < rightEntry.Specification.Order;

				return leftEntry.Information.Id < rightEntry.Information.Id;

			};

		};


	};
	

	// 本质是 Schedule 的阶段筛，一种存储结构
	class SchedulePhase {

	public:

		SchedulePhase& AddSystem(const PhaseSystemEntry& phaseSystemEntry);

		template<typename System>
			requires std::derived_from<System, ISystem>
		inline SchedulePhase& RemoveSystem() {

			SystemID id = GetSystemID<System>();

			auto it = std::ranges::find_if(m_OrderedSystems,
				[&id](const PhaseSystemEntry& entry) -> bool {return entry.Id == id; }
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

		void Run(const SystemContext& context);


		template<typename System>
			requires std::derived_from<System, ISystem>&& std::default_initializable<System>
		inline SystemSchedule& AddSystem(const std::initializer_list<SystemPhase>& phases, const std::initializer_list<SceneMode>& modes) {	//! “缩写函数模板”或“简写函数模板”

			auto it = FindSystem<System>();

			if (it != m_Systems.end())
				return *this;

			//! 系统插入
			SystemPhase phaseMask = SystemPhase::None;
			for (const SystemPhase phase : phases) {
				phaseMask |= phase;
			}

			SceneMode modeMask = SceneMode::None;
			for (const SceneMode mode : modes) {
				modeMask |= mode;
			}
			
			int systemsCount = m_Systems.size();	// 自动 Order 计数

			m_Systems.emplace_back(
				SystemEntry{ 
					.Information{.Id{GetSystemID<System>()}, .Name{typeid(System).name()}},
					.Specification{.PhaseMask{phaseMask}, .ModeMask{modeMask}, .Order{m_NextOrder}},
					.Instance{CreateScope<System>()},
				});
			m_Systems.back().Instance->OnAttach();

			if (systemsCount != m_Systems.size())
				m_NextOrder++;

			//! 阶段编辑
			InsertSystemToPhase((PhaseSystemEntry)(m_Systems.back()), phases);


			return *this;

		}

		template<typename System>
			requires std::derived_from<System, ISystem>&& std::default_initializable<System>
		inline SystemSchedule& RemoveSystem() {

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
			requires std::derived_from<System, ISystem>&& std::default_initializable<System>
		inline bool Contain() const { return FindSystem<System>() != m_Systems.end(); };

		inline bool Contain(SystemID systemId) const { return FindSystem(systemId) != m_Systems.end(); };

	private:

		template<typename System>
			requires std::derived_from<System, ISystem>
		inline std::vector<SystemEntry>::iterator FindSystem() {

			auto it = std::ranges::find_if(
				m_Systems,
				[](const SystemEntry& entry) ->bool {
					return entry.Information.Id == GetSystemID<System>();
				}
			);

			return it;
		};

		template<typename System>
			requires std::derived_from<System, ISystem>
		inline std::vector<SystemEntry>::const_iterator FindSystem() const {

			auto it = std::ranges::find_if(
				m_Systems,
				[](const SystemEntry& entry)->bool {
					return entry.Information.Id == GetSystemID<System>();
				}
			);

			return it;
		};

		std::vector<SystemEntry>::iterator FindSystem(SystemID systemId);

		std::vector<SystemEntry>::const_iterator FindSystem(SystemID systemId) const;


	private:

		void InsertSystemToPhase(const PhaseSystemEntry& phaseSystemEntry, const std::initializer_list<SystemPhase>& phases);

	private:

		std::vector<SystemEntry> m_Systems;

		int m_NextOrder = 0;

		SchedulePhase m_PreUpdate_Phase;
		SchedulePhase m_Update_Phase;
		SchedulePhase m_PostUpdate_Phase;
		SchedulePhase m_Presentation_Phase;

	};
};
