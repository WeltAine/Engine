#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/System/SystemTypes.h"
#include "Ayin/System/SystemRegistry.h"

#include <concepts>
#include <string_view>
#include <vector>


namespace Ayin {

	class SystemSchedule;

	class AYIN_API SystemPipeline {

	public:

		class AYIN_API Builder {
		public:
			using DefinitionList = std::vector<SystemDefinition>;

		private:
			DefinitionList m_Definitions;
			int m_NextOrder = 0;

		public:

			Builder() = default;
			Builder(const SystemPipeline& pipeline);

			SystemPipeline Build() const;

			Builder& AddSystem(const SystemDefinition& definition);

			template<typename System>
				requires std::derived_from<System, ISystem>&& std::default_initializable<System>
			Builder& AddSystem(const std::vector<SystemPhase>& phases, const std::vector<SceneMode>& modes);

			template<typename System>
				requires std::derived_from<System, ISystem>&& std::default_initializable<System>
			Builder& AddSystem(const std::vector<SystemPhase>& phases, const std::vector<SceneMode>& modes, int order);


			Builder& RemoveSystem(SystemID systemId);
			Builder& RemoveSystem(std::string_view typeKey);
			Builder& RemoveSystemPhase(SystemID systemId, SystemPhase phase);
			Builder& SetSystemSpecification(SystemID systemId, const SystemSpecification& specification);
			Builder& SetSystemConfiguration(SystemID systemId, const SystemConfiguration& configuration);

			inline const DefinitionList& GetDefinitions() const { return m_Definitions; };

			bool ContainSystem(SystemID systemId) const;
			bool ContainSystem(std::string_view typeKey) const;

		private:

			DefinitionList::iterator FindSystem(SystemID systemId);
			DefinitionList::iterator FindSystem(std::string_view typeKey);
			DefinitionList::const_iterator FindSystem(SystemID systemId) const;
			DefinitionList::const_iterator FindSystem(std::string_view typeKey) const;
			void SortDefinitions();

		};

	private:

		std::vector<SystemDefinition> m_Definitions;
		bool m_Valid = true;		// pipeline 是否有效

	private:

		bool Build(SystemSchedule& schedule) const;

	public:

		SystemSchedule CreateSchedule() const;
		inline bool IsValid() const { return m_Valid; };
		inline const std::vector<SystemDefinition>& GetDefinitions() const { return m_Definitions; };

	};


	template<typename System>
		requires std::derived_from<System, ISystem>&& std::default_initializable<System>
	inline SystemPipeline::Builder& SystemPipeline::Builder::AddSystem(
		const std::vector<SystemPhase>& phases,
		const std::vector<SceneMode>& modes) {

		//! 不能再使用 std::initializer_list，因为其底层记录的是指针，而非具体元素，我们会延迟使用，即使是对 std::initializer_list 进行值拷贝也没法复制到值
		return AddSystem<System>(phases, modes, m_NextOrder);

	};

	template<typename System>
		requires std::derived_from<System, ISystem>&& std::default_initializable<System>
	inline SystemPipeline::Builder& SystemPipeline::Builder::AddSystem(
		const std::vector<SystemPhase>& phases,
		const std::vector<SceneMode>& modes,
		const int order) {

		SystemPhase phaseMask = SystemPhase::None;
		for (const SystemPhase phase : phases)
			phaseMask |= phase;

		SceneMode modeMask = SceneMode::None;
		for (const SceneMode mode : modes)
			modeMask |= mode;


			
		// 确保 System 已经注册到 SystemRegistry 中

		const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(GetSystemID<System>());
		if (descriptor == nullptr) {
			AYIN_CORE_ERROR("System RuntimeId is not registered");
			return *this;
		}

		return AddSystem(SystemDefinition{
			.Type{descriptor->TypeKey},
			.Specification{.PhaseMask{phaseMask}, .ModeMask{modeMask}, .Order{order}},
			.Configuration{}
		});
	};


};