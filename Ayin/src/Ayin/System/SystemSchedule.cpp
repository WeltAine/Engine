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
