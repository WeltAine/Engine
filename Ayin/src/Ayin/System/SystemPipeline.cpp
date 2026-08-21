#include <AyinPch.h>

#include "Ayin/System/SystemPipeline.h"
#include "Ayin/System/SystemSchedule.h"

#include <algorithm>
#include <unordered_set>


namespace Ayin {

	SystemPipeline::Builder::Builder(const SystemPipeline& pipeline)
		: m_Definitions{ pipeline.m_Definitions } {

		for (const SystemDefinition& definition : m_Definitions)
			m_NextOrder = std::max(m_NextOrder, definition.Specification.Order + 1);

	};


	void SystemPipeline::Builder::SortDefinitions() {

		std::stable_sort(
			m_Definitions.begin(),
			m_Definitions.end(),
			[](const SystemDefinition& left, const SystemDefinition& right) {
				return left.Specification.Order < right.Specification.Order;
			});

	};


	SystemPipeline SystemPipeline::Builder::Build() {

		std::unordered_set<SystemTypeKey> types;
		DefinitionList definitions;
		definitions.reserve(m_Definitions.size());

		for (const SystemDefinition& definition : m_Definitions) {
			if (!types.emplace(definition.Type).second) {
				AYIN_CORE_ERROR("System '{}' appears more than once in the Pipeline", definition.Type);
				continue;
			}

			if (SystemRegistry::GetSystemDescriptor(definition.Type) == nullptr) {
				AYIN_CORE_ERROR("System '{}' is not registered", definition.Type);
				continue;
			}

			definitions.emplace_back(definition);
		}

		std::stable_sort(
			definitions.begin(),
			definitions.end(),
			[](const SystemDefinition& left, const SystemDefinition& right) {
				return left.Specification.Order < right.Specification.Order;
			});

		SystemPipeline pipeline;
		pipeline.m_Definitions = std::move(definitions);
		return pipeline;

	};


	SystemPipeline::Builder& SystemPipeline::Builder::AddSystem(const SystemDefinition& definition) {

		const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(definition.Type);
		if (descriptor == nullptr) {
			AYIN_CORE_ERROR("System '{}' is not registered", definition.Type);
			return *this;
		}

		if (ContainSystem(descriptor->RuntimeId)) {
			AYIN_CORE_ERROR("System '{}' has already been added", descriptor->TypeKey);
			return *this;
		}

		SystemDefinition canonicalDefinition = definition;
		canonicalDefinition.Type = descriptor->TypeKey;
		if (canonicalDefinition.Specification.Order < 0)
			canonicalDefinition.Specification.Order = m_NextOrder;

		m_NextOrder = std::max(m_NextOrder, canonicalDefinition.Specification.Order + 1);
		m_Definitions.emplace_back(std::move(canonicalDefinition));
		SortDefinitions();

		return *this;

	};


	SystemPipeline::Builder& SystemPipeline::Builder::RemoveSystem(const SystemID systemId) {

		const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(systemId);
		if (descriptor != nullptr)
			RemoveSystem(descriptor->TypeKey);

		return *this;

	};

	SystemPipeline::Builder& SystemPipeline::Builder::RemoveSystem(const std::string_view typeKey) {

		auto it = FindSystem(typeKey);
		if (it != m_Definitions.end())
			m_Definitions.erase(it);

		return *this;

	};

	SystemPipeline::Builder& SystemPipeline::Builder::RemoveSystemPhase(const SystemID systemId, const SystemPhase phase) {

		auto it = FindSystem(systemId);
		if (it != m_Definitions.end())
			it->Specification.PhaseMask &= ~phase;

		return *this;

	};

	SystemPipeline::Builder& SystemPipeline::Builder::SetSystemSpecification(
		const SystemID systemId,
		const SystemSpecification& specification) {

		auto it = FindSystem(systemId);
		if (it == m_Definitions.end())
			return *this;

		it->Specification = specification;
		if (it->Specification.Order < 0)
			it->Specification.Order = m_NextOrder++;
		SortDefinitions();

		return *this;

	};


	SystemPipeline::Builder::DefinitionList::iterator SystemPipeline::Builder::FindSystem(const SystemID systemId) {

		const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(systemId);
		return descriptor == nullptr
			? m_Definitions.end()
			: FindSystem(descriptor->TypeKey);

	};

	SystemPipeline::Builder::DefinitionList::iterator SystemPipeline::Builder::FindSystem(const std::string_view typeKey) {

		return std::ranges::find_if(
			m_Definitions,
			[typeKey](const SystemDefinition& definition) {
				return definition.Type == typeKey;
			});

	};

	SystemPipeline::Builder::DefinitionList::const_iterator SystemPipeline::Builder::FindSystem(const SystemID systemId) const {

		const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(systemId);
		return descriptor == nullptr
			? m_Definitions.end()
			: FindSystem(descriptor->TypeKey);

	};

	SystemPipeline::Builder::DefinitionList::const_iterator SystemPipeline::Builder::FindSystem(const std::string_view typeKey) const {

		return std::ranges::find_if(
			m_Definitions,
			[typeKey](const SystemDefinition& definition) {
				return definition.Type == typeKey;
			});

	};


	bool SystemPipeline::Builder::ContainSystem(const SystemID systemId) const {
		return FindSystem(systemId) != m_Definitions.end();
	};

	bool SystemPipeline::Builder::ContainSystem(const std::string_view typeKey) const {
		return FindSystem(typeKey) != m_Definitions.end();
	};


	void SystemPipeline::Build(SystemSchedule& schedule) const {

		for (const SystemDefinition& definition : m_Definitions)
			schedule.AddSystem(definition);

	};

	SystemSchedule SystemPipeline::CreateSchedule() const {

		SystemSchedule schedule{};
		Build(schedule);
		return schedule;

	};


};
