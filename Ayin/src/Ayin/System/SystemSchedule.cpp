#include<AyinPch.h>

#include"Ayin/System/SystemSchedule.h"

namespace Ayin {

	SchedulePhase& SchedulePhase::AddSystem(const PhaseSystemEntry& phaseSystemEntry) {	//! “缩写函数模板”或“简写函数模板”

		auto it = std::ranges::find_if(m_OrderedSystems, 
			[&phaseSystemEntry](const PhaseSystemEntry& entry) -> bool {
				return entry.Id == phaseSystemEntry.Id; 
			});

		if (it != m_OrderedSystems.end())
			return *this;

		m_OrderedSystems.insert(phaseSystemEntry);
		return *this;

	};

	SchedulePhase& SchedulePhase::RemoveSystem(const SystemID systemId) {

		auto it = std::ranges::find_if(m_OrderedSystems,
			[&systemId](const PhaseSystemEntry& entry) -> bool {return entry.Id == systemId; }
		);

		if (it != m_OrderedSystems.end())
			m_OrderedSystems.erase(it);

		return *this;

	};

};

namespace Ayin {


	SystemSchedule::~SystemSchedule() {

		// OnAttach 按注册顺序执行，因此析构时以相反顺序解除系统。
		std::ranges::for_each(
			m_Systems | std::views::reverse,
			[](const SystemEntry& entry) -> void {
				entry.Instance->OnDetach();
			});

	};



	void SystemSchedule::Begin(const SystemContext& systemContext) {

		m_BegunSystems.clear();

		std::vector<SystemEntry*> runnableSystems;
		for (SystemEntry& entry : m_Systems) {
			if (static_cast<bool>(entry.Specification.ModeMask & systemContext.Mode))
				runnableSystems.emplace_back(&entry);
		}

		// 相同 Order 保留注册顺序；不改变 m_Systems 自身的所有权和注册顺序。
		std::stable_sort(
			runnableSystems.begin(), runnableSystems.end(),
			[](const SystemEntry* leftEntry, const SystemEntry* rightEntry) -> bool {
				return leftEntry->Specification.Order < rightEntry->Specification.Order;
			});

		for (SystemEntry* entry : runnableSystems) {
			m_BegunSystems.emplace_back(entry->Information.Id);
			entry->Instance->OnBegin(systemContext);
		}

	};



	void SystemSchedule::Run(const SystemContext& context) {

		SystemContext phaseContext = context;


		auto prePhase_Update = [&phaseContext](SchedulePhase& phase) -> void {

			std::ranges::for_each(phase, [&phaseContext](const PhaseSystemEntry& entry) -> void
				{
					const_cast<PhaseSystemEntry&>(entry).OnPreUpdate(phaseContext);
				});

			};

		auto updatePhase_Update = [&phaseContext](SchedulePhase& phase) -> void {

			std::ranges::for_each(phase, [&phaseContext](const PhaseSystemEntry& entry) -> void
				{
					const_cast<PhaseSystemEntry&>(entry).OnUpdate(phaseContext);
				});

			};

		auto postPhase_Update = [&phaseContext](SchedulePhase& phase) -> void {

			std::ranges::for_each(phase, [&phaseContext](const PhaseSystemEntry& entry) -> void
				{
					const_cast<PhaseSystemEntry&>(entry).OnPostUpdate(phaseContext);
				});

			};

		auto presentationPhase_Update = [&phaseContext](SchedulePhase& phase) -> void {

			std::ranges::for_each(phase, [&phaseContext](const PhaseSystemEntry& entry) -> void
				{
					const_cast<PhaseSystemEntry&>(entry).OnPresentationUpdate(phaseContext);
				});

			};




		phaseContext.Phase = SystemPhase::PreUpdate;
		prePhase_Update(m_PreUpdate_Phase);

		phaseContext.Phase = SystemPhase::Update;
		updatePhase_Update(m_Update_Phase);

		phaseContext.Phase = SystemPhase::PostUpdate;
		postPhase_Update(m_PostUpdate_Phase);

		phaseContext.Phase = SystemPhase::Presentation;
		presentationPhase_Update(m_Presentation_Phase);

	}


	void SystemSchedule::End(const SystemContext& systemContext) {

		// 与 Begin 相反的顺序结束，只处理本次真正执行过 OnBegin 的系统。
		std::ranges::for_each(
			m_BegunSystems | std::views::reverse,
			[this, &systemContext](const SystemID& systemId) -> void {

				auto entry = this->FindSystem(systemId);
				if (entry != this->m_Systems.end())
					entry->Instance->OnEnd(systemContext);

			});

		m_BegunSystems.clear();

	};


	std::vector<SystemEntry>::iterator SystemSchedule::FindSystem(SystemID systemId) {

		auto it = std::ranges::find_if(
			m_Systems,
			[&systemId](const SystemEntry& entry) ->bool {
				return entry.Information.Id == systemId;
			}
		);

		return it;
	};

	std::vector<SystemEntry>::const_iterator SystemSchedule::FindSystem(SystemID systemId) const {

		auto it = std::ranges::find_if(
			m_Systems,
			[&systemId](const SystemEntry& entry)->bool {
				return entry.Information.Id == systemId;
			}
		);

		return it;
	};

	void SystemSchedule::InsertSystemToPhase(const PhaseSystemEntry& phaseSystemEntry, const std::initializer_list<SystemPhase>& phases) {

		for (const SystemPhase& phase : phases) {
			switch (phase) {

			case(SystemPhase::PreUpdate): m_PreUpdate_Phase.AddSystem(phaseSystemEntry); break;;
			case(SystemPhase::Update): m_Update_Phase.AddSystem(phaseSystemEntry); break;;
			case(SystemPhase::PostUpdate): m_PostUpdate_Phase.AddSystem(phaseSystemEntry); break;
			case(SystemPhase::Presentation): m_Presentation_Phase.AddSystem(phaseSystemEntry); break;
			default: break;

			}
		}

	};

};
