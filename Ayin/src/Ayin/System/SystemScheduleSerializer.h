#pragma once

#include"Ayin/Core/Core.h"

#include "Ayin/System/SystemSchedule.h"
#include "Ayin/System/SystemPipeline.h"

namespace Ayin {

	struct SystemJson {

		std::string Name;

		std::vector<SystemPhase> Phases;

		std::vector<SceneMode> Modes;

		int Order = -1;											// -1 表示不指定顺序（将由程序以自动递加的顺序设定）

		::glz::raw_json SystemData = NullSystemData;

		static constexpr const char* NullSystemData = "{}";

		struct glaze {
			using T = SystemJson;
			static constexpr auto value = glz::object(
				"Name", &T::Name,
				"Phases", &T::Phases,
				"Modes", &T::Modes,
				"Order", &T::Order,
				"SystemData", &T::SystemData);
		};


		operator SystemRegistration() const {
		
			return SystemRegistration{
				.Information{SystemRegistry::GetSystemDescriptor(Name)->Information},
				.Specification{
					.PhaseMask{Synthesis(Phases)},
					.ModeMask{Synthesis(Modes)},
					.Order{Order}
				},
				.SystemData{SystemData}
			};


		};

	};


	struct SystemPipelineJson {

		std::vector<SystemJson> Systems;

		struct glaze {
			using T = SystemPipelineJson;
			static constexpr auto value = glz::object(
				"Systems", &T::Systems
			);
		};

	};

};


namespace Ayin {

	//! 因为序列化的来源可能是 schedule 或者 pipeline ，所以决定做成系统，而不是序列化器，用于提供序列化中间机构，对于 WorldSerializer 来说这才是真正重要的
	class AYIN_API SystemScheduleSerializer {

	public:

		static SystemPipeline::Builder Deserializer(const SystemPipelineJson& systemPipelineJson);


		static std::optional<SystemPipelineJson> BuildSystemPipelineJson(const SystemSchedule& schedule);
		static std::optional<SystemPipelineJson> BuildSystemPipelineJsonFrom(std::string_view jsonStr);
		

	};

};
