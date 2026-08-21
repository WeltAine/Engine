#include <AyinPch.h>

#include "Ayin/System/SystemPipeline.h"
#include "Ayin/System/SystemSchedule.h"
#include "Ayin/System/SystemScheduleSerializer.h"
#include "Ayin/Core/BitmaskEnum.h"
#include <algorithm>


namespace Ayin {



	SystemRegistration::operator SystemJson() const
	{
		return SystemJson{
			.Name{Information.Name},
			.Phases{Disassemble(Specification.PhaseMask)},
			.Modes{Disassemble(Specification.ModeMask)},
			.Order{Specification.Order},
			.SystemData{SystemData}
		};
	};



	// ----------------------------------------------------------------------------

	SystemPipeline::Builder::Builder(const SystemPipeline& pipeline)
		: m_Registrations{ pipeline.m_Registrations } {

		for (const SystemRegistration& registration : m_Registrations)
			m_NextOrder = std::max(m_NextOrder, registration.Specification.Order + 1);

	};

	SystemPipeline SystemPipeline::Builder::Build() {
	
		SystemPipeline pipeline;

		pipeline.m_Registrations = std::move(m_Registrations);

		return pipeline;

	};

	SystemPipeline::Builder& SystemPipeline::Builder::AddSystem(const SystemRegistration& systemRegistration) {
	
		// 获取描述符（正确的系统描述）(当不匹配时以 名称 为准)
		const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(systemRegistration.Information.RuntimeId);
		if (descriptor == nullptr) {
			descriptor = SystemRegistry::GetSystemDescriptor(systemRegistration.Information.Name);
		}
		else if (descriptor->Information.Name != systemRegistration.Information.Name) {
			descriptor = SystemRegistry::GetSystemDescriptor(systemRegistration.Information.Name);
		}

		if (descriptor == nullptr) {
			AYIN_CORE_ERROR("System '{}' is not registered", systemRegistration.Information.Name);
			return *this;
		}


		RemoveSystem(systemRegistration.Information.Name);

		m_Registrations.emplace(systemRegistration);

		m_NextOrder = std::max(m_NextOrder, systemRegistration.Specification.Order + 1);

		return *this;
		
	};

	SystemPipeline::Builder& SystemPipeline::Builder::RemoveSystem(const SystemID systemId) {

		if (ContainSystem(systemId)) {

			auto it = FindSystem(systemId);
			m_Registrations.erase(it);

		}

		return *this;

	};

	SystemPipeline::Builder& SystemPipeline::Builder::RemoveSystem(const std::string_view systemName) {

		if (ContainSystem(systemName)) {

			auto it = FindSystem(systemName);
			m_Registrations.erase(it);

		}

		return *this;

	};
	SystemPipeline::Builder& SystemPipeline::Builder::RemoveSystemPhase(SystemID systemId, SystemPhase phase) {
	
		auto it = FindSystem(systemId);

		if (it == m_Registrations.end())
			return *this;

		SystemRegistration registration = *it;
		registration.Specification.PhaseMask &= ~(phase);

		m_Registrations.erase(it);
		m_Registrations.emplace(registration);

		return *this;
	
	};

	SystemPipeline::Builder& SystemPipeline::Builder::SetSystemSpecification(SystemID systemId, const SystemSpecification& specification) {
	
		auto it = FindSystem(systemId);

		if (it == m_Registrations.end())
			return *this;

		SystemRegistration registration = *it;
		registration.Specification = specification;

		m_Registrations.erase(it);
		m_Registrations.emplace(registration);

		return *this;

	};


	SystemPipeline::Builder::RegistrationSet::iterator SystemPipeline::Builder::FindSystem(SystemID systemId) {
	
		auto it = std::ranges::find_if(m_Registrations,
			[systemId](const SystemRegistration& systemRegistration) -> bool {
				if (systemRegistration.Information.RuntimeId == systemId) return true;
				return false;
			});

		return it;

	};
	SystemPipeline::Builder::RegistrationSet::iterator SystemPipeline::Builder::FindSystem(const std::string_view systemName) {
	
		auto it = std::ranges::find_if(m_Registrations,
			[systemName](const SystemRegistration& systemRegistration) -> bool {
				if (systemRegistration.Information.Name == systemName) return true;
				return false;
			});

		return it;

	};
	SystemPipeline::Builder::RegistrationSet::const_iterator SystemPipeline::Builder::FindSystem(SystemID systemId) const {

		auto it = std::ranges::find_if(m_Registrations,
			[systemId](const SystemRegistration& systemRegistration) -> bool {
				if (systemRegistration.Information.RuntimeId == systemId) return true;
				return false;
			});

		return it;

	};
	SystemPipeline::Builder::RegistrationSet::const_iterator SystemPipeline::Builder::FindSystem(const std::string_view systemName) const {

		auto it = std::ranges::find_if(m_Registrations,
			[systemName](const SystemRegistration& systemRegistration) -> bool {
				if (systemRegistration.Information.Name == systemName) return true;
				return false;
			});

		return it;

	};


	bool SystemPipeline::Builder::ContainSystem(SystemID systemId) const {
		return FindSystem(systemId) != m_Registrations.end();
	};
	bool SystemPipeline::Builder::ContainSystem(const std::string_view systemName) const {
		return FindSystem(systemName) != m_Registrations.end();
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
