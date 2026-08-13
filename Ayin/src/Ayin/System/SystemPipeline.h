#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/System/Systems.h"

#include "Ayin/System/SystemSchedule.h"

namespace Ayin {

	
	class AYIN_API SystemPipeline {

	public:

		class AYIN_API Builder {

		private:
			std::vector<std::function<void(SystemSchedule& schedule)>> m_Registrations;
			int m_NextOrder = 0;

		public:

			SystemPipeline Build();


			template<typename System>
				requires std::derived_from<System, ISystem>&& std::default_initializable<System>
			inline Builder& AddSystem(const std::vector<SystemPhase>& phases, const std::vector<SceneMode>& modes) {
				//! 不能再使用 std::initializer_list，因为其底层记录的是指针，而非具体元素，我们会延迟使用，即使是对 std::initializer_list 进行值拷贝也没法复制到值
				const int order = m_NextOrder++;
				
				m_Registrations.emplace_back(
					[order, phases, modes](SystemSchedule& schedule) -> void {
					
						schedule.AddSystem<System>(phases, modes, order);

					});

				return *this;

			};

			template<typename System>
				requires std::derived_from<System, ISystem>&& std::default_initializable<System>
			inline Builder& AddSystem(const std::vector<SystemPhase>& phases, const std::vector<SceneMode>& modes, int order) {

				m_Registrations.emplace_back(
					[order, phases, modes](SystemSchedule& schedule) -> void {

						schedule.AddSystem<System>(phases, modes, order);

					});

				m_NextOrder = std::max(m_NextOrder, order + 1);

				return *this;

			};

		};

	private:

		std::vector<std::function<void(SystemSchedule& schedule)>> m_Registrations;

	public:

		void BuildSchedule(SystemSchedule& schedule) const;
		SystemSchedule CreateSchedule() const;

	};


};
