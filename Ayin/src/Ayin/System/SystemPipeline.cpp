#include <AyinPch.h>

#include "Ayin/System/SystemPipeline.h"
#include <algorithm>


namespace Ayin {


	SystemPipeline SystemPipeline::Builder::Build() {
	
		SystemPipeline pipeline;

		pipeline.m_Registrations = std::move(m_Registrations);

		return pipeline;

	};

	// ---------------------------------------------------------------------------------------


	void SystemPipeline::BuildSchedule(SystemSchedule& schedule) const {
	
		std::ranges::for_each(
			m_Registrations,
			[&schedule](const std::function<void(SystemSchedule& schedule)>& func) -> void {
				func(schedule);
			});

	};

	SystemSchedule SystemPipeline::CreateSchedule() const {
	
		SystemSchedule schedule{};

		BuildSchedule(schedule);

		return schedule;

	};


};
