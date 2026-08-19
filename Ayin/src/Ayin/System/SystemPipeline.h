#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/System/Systems.h"

#include "Ayin/System/SystemSchedule.h"

namespace Ayin {



	struct AYIN_API SystemRegistration {

		SystemInformation Information;
		SystemSpecification Specification;

		::glz::raw_json SystemData = NullSystemData;

		static constexpr const char* NullSystemData = "{}";

	};

	
	class AYIN_API SystemPipeline {

		friend class SystemPipelineSerializer;

	public:

		class AYIN_API Builder {

		private:
			std::vector<SystemRegistration> m_Registrations;
			int m_NextOrder = 0;

		public:

			SystemPipeline Build();


			template<typename System>
				requires std::derived_from<System, ISystem>&& std::default_initializable<System>
			inline Builder& AddSystem(const std::vector<SystemPhase>& phases, const std::vector<SceneMode>& modes) {
				//! 不能再使用 std::initializer_list，因为其底层记录的是指针，而非具体元素，我们会延迟使用，即使是对 std::initializer_list 进行值拷贝也没法复制到值
				const int order = m_NextOrder++;

				return AddSystem<System>(phases, modes, order);

			};

			template<typename System>
				requires std::derived_from<System, ISystem>&& std::default_initializable<System>
			inline Builder& AddSystem(const std::vector<SystemPhase>& phases, const std::vector<SceneMode>& modes, int order) {

				SystemPhase phaseMask = SystemPhase::None;
				for (const SystemPhase phase : phases) {
					phaseMask |= phase;
				}

				SceneMode modeMask = SceneMode::None;
				for (const SceneMode mode : modes) {
					modeMask |= mode;
				}

				SystemRegistration systemRegistration{
					.Information{.Name{typeid(System).nams()}, .RuntimeId{GetSystemID<System>()}},
					.Specification{.PhaseMask{phaseMask}, .ModeMask{modeMask}, .Order{order}}
				};

				m_Registrations.emplace_back(systemRegistration);

				m_NextOrder = std::max(m_NextOrder, order + 1);

				return *this;

			};


			Builder& AddSystem(const SystemRegistration& systemRegistration);
			Builder& RemoveSystem(const SystemID systemId);
			Builder& RemoveSystem(const std::string systemName);

		private:

			std::vector<SystemRegistration>::iterator FindSystem(SystemID systemId);
			std::vector<SystemRegistration>::iterator FindSystem(const std::string& systemName);
			std::vector<SystemRegistration>::const_iterator FindSystem(SystemID systemId) const;
			std::vector<SystemRegistration>::const_iterator FindSystem(const std::string& systemName) const;
			bool ContainSystem(SystemID systemId) const;
			bool ContainSystem(const std::string& systemName) const;

		};

	private:

		std::vector<SystemRegistration> m_Registrations;

	public:

		void Build(SystemSchedule& schedule) const;
		SystemSchedule CreateSchedule() const;

	};


};
