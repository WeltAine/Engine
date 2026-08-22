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

		bool valid = true;
		for (const SystemDefinition& definition : m_Definitions) {

			// Build 不再静默丢弃非法定义；候选 Pipeline 必须完整有效才能提交。
			if (!types.emplace(definition.Type).second) {
				AYIN_CORE_ERROR("System '{}' appears more than once in the Pipeline", definition.Type);
				valid = false;
				continue;
			}

			if (SystemRegistry::GetSystemDescriptor(definition.Type) == nullptr) {
				AYIN_CORE_ERROR("System '{}' is not registered", definition.Type);
				valid = false;
				continue;
			}

			definitions.emplace_back(definition);
		}

		// 排序
		std::stable_sort(
			definitions.begin(),
			definitions.end(),
			[](const SystemDefinition& left, const SystemDefinition& right) {
				return left.Specification.Order < right.Specification.Order;
			});

		SystemPipeline pipeline;
		pipeline.m_Definitions = std::move(definitions);
		pipeline.m_Valid = valid && pipeline.m_Definitions.size() == m_Definitions.size();
		return pipeline;

	};


	SystemPipeline::Builder& SystemPipeline::Builder::AddSystem(const SystemDefinition& definition) {

		const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(definition.Type);
		if (descriptor == nullptr) {
			AYIN_CORE_ERROR("System '{}' is not registered", definition.Type);
			return *this;
		}

		if (ContainSystem(descriptor->RuntimeId)) {	// 防止重复添加
			AYIN_CORE_ERROR("System '{}' has already been added", descriptor->TypeKey);
			return *this;
		}

		SystemDefinition canonicalDefinition = definition;
		canonicalDefinition.Type = descriptor->TypeKey;
		if (canonicalDefinition.Specification.Order < 0)
			canonicalDefinition.Specification.Order = m_NextOrder;

		m_NextOrder = std::max(m_NextOrder, canonicalDefinition.Specification.Order + 1);
		m_Definitions.emplace_back(std::move(canonicalDefinition));
		SortDefinitions();		//? 这个阶段的排序是必要的么？
		//! 这个排序不是 Schedule 运行所必须的。因为即使 Builder 内部暂时保持添加顺序，Build() 时也可以统一按照 Order 排序，最终 Schedule 仍然能够得到正确顺序。
		//! 但是，对于当前 Builder 的语义，它又是有必要的。
		//! Builder 不只是一个构建过程中的临时容器，它之后还要直接服务于 Editor 或者其它东西
		//! 它们读取的应该是当前 Builder 的结构顺序

		//! 此外，如果排序延迟到 Build()
		//! GetDefinitions() 暴露的是一个未规范化状态；
		//! Builder 的调用者无法判断当前容器顺序是否可信。

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

	SystemPipeline::Builder& SystemPipeline::Builder::SetSystemConfiguration(
		const SystemID systemId,
		const SystemConfiguration& configuration) {

		auto it = FindSystem(systemId);
		if (it != m_Definitions.end())
			it->Configuration = configuration;

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


	bool SystemPipeline::Build(SystemSchedule& schedule) const {

		if (!m_Valid)
			return false;

		if (!schedule.BeginConstruction())
			return false;

		for (const SystemDefinition& definition : m_Definitions) {
			if (!schedule.BuildSystem(definition)) {
				schedule.ClearSystems();
				return false;
			}
		}

		return schedule.FinishConstruction();

	};

	SystemSchedule SystemPipeline::CreateSchedule() const {

		SystemSchedule schedule{};
		Build(schedule);
		return schedule;

	};


};