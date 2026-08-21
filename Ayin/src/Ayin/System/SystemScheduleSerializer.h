#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Serialization/Json/EngineEnumJson.h"
#include "Ayin/System/SystemPipeline.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Ayin {

	struct SystemJson {

		std::string Name;

		std::vector<SystemPhase> Phases;
		std::vector<SceneMode> Modes;

		int Order = -1;
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

	};

	// 系统管线序列化反序列化中间结构
	struct SystemPipelineJson {
		
		std::vector<SystemJson> Systems;
		
		struct glaze {
			using T = SystemPipelineJson;
			static constexpr auto value = glz::object(
				"Systems", &T::Systems
			);
		};
		
	};
	


	class SystemSchedule;

	//! 因为序列化的来源可能是 schedule 或者 pipeline ，所以决定做成系统，而不是序列化器，用于提供序列化中间机构，对于 WorldSerializer 来说这才是真正重要的
	class AYIN_API SystemScheduleSerializer {

	public:

		static SystemPipeline::Builder Deserializer(const SystemPipelineJson& systemPipelineJson);

		static std::optional<SystemPipelineJson> BuildSystemPipelineJson(const SystemSchedule& schedule);
		static std::optional<SystemPipelineJson> BuildSystemPipelineJsonFrom(std::string_view jsonStr);

	};


};
