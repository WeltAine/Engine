#include <AyinPch.h>

#include "Ayin/System/SystemPipeline.h"
#include <algorithm>


namespace Ayin {


	SystemPipeline SystemPipeline::Builder::Build() {
	
		SystemPipeline pipeline;

		pipeline.m_Registrations = std::move(m_Registrations);

		return pipeline;

	};

	SystemPipeline::Builder& SystemPipeline::Builder::AddSystem(const SystemRegistration& systemRegistration) {
	
		RemoveSystem(systemRegistration.Information.Name);

		m_Registrations.emplace_back(systemRegistration);

		return *this;
		
	};

	SystemPipeline::Builder& SystemPipeline::Builder::RemoveSystem(const SystemID systemId) {

		if (ContainSystem(systemId)) {

			auto it = FindSystem(systemId);
			m_Registrations.erase(it);

		}

		return *this;

	};

	SystemPipeline::Builder& SystemPipeline::Builder::RemoveSystem(const std::string systemName) {

		if (ContainSystem(systemName)) {

			auto it = FindSystem(systemName);
			m_Registrations.erase(it);

		}

		return *this;

	};



	std::vector<SystemRegistration>::iterator SystemPipeline::Builder::FindSystem(SystemID systemId) {
	
		auto it = std::ranges::find_if(m_Registrations,
			[systemId](const SystemRegistration& systemRegistration) -> bool {
				if (systemRegistration.Information.RuntimeId == systemId) return true;
				return false;
			});

		return it;

	};
	std::vector<SystemRegistration>::iterator SystemPipeline::Builder::FindSystem(const std::string& systemName) {
	
		auto it = std::ranges::find_if(m_Registrations,
			[systemName](const SystemRegistration& systemRegistration) -> bool {
				if (systemRegistration.Information.Name == systemName) return true;
				return false;
			});

		return it;

	};
	std::vector<SystemRegistration>::const_iterator SystemPipeline::Builder::FindSystem(SystemID systemId) const {

		auto it = std::ranges::find_if(m_Registrations,
			[systemId](const SystemRegistration& systemRegistration) -> bool {
				if (systemRegistration.Information.RuntimeId == systemId) return true;
				return false;
			});

		return it;

	};
	std::vector<SystemRegistration>::const_iterator SystemPipeline::Builder::FindSystem(const std::string& systemName) const {

		auto it = std::ranges::find_if(m_Registrations,
			[systemName](const SystemRegistration& systemRegistration) -> bool {
				if (systemRegistration.Information.Name == systemName) return true;
				return false;
			});

		return it;

	};


	bool SystemPipeline::Builder::ContainSystem(SystemID systemId) const {
		if (FindSystem(systemId) != m_Registrations.end()) return true;
	};
	bool SystemPipeline::Builder::ContainSystem(const std::string& systemName) const {
		if (FindSystem(systemName) != m_Registrations.end()) return true;
	};



	// ---------------------------------------------------------------------------------------


	void SystemPipeline::Build(SystemSchedule& schedule) const {
	
		std::ranges::for_each(
			m_Registrations,
			[&schedule](const SystemRegistration& systemRegistration) -> void {
				schedule.AddSystem(systemRegistration);
			});

	};

	SystemSchedule SystemPipeline::CreateSchedule() const {
	
		SystemSchedule schedule{};

		Build(schedule);

		return schedule;

	};


};
