#include <AyinPch.h>

#include "Ayin/System/SystemScheduleSerializer.h"

namespace Ayin {
	
	// ---------------------------------------------- 辅助方法 -------------------------------------------------------

	SystemJson BuildSystemJson(const SystemRegistration& registration) {

		return SystemJson{
			.Name{registration.Information.Name},
			.Phases{Disassemble(registration.Specification.PhaseMask)},
			.Modes{Disassemble(registration.Specification.ModeMask)},
			.Order{registration.Specification.Order},
			.SystemData{registration.SystemData}
		};


	};

	//? 有必要么？我们该从 Entry 中反序列化么
	SystemJson BuildSystemJson(const SystemEntry& entry) {

		return SystemJson{
			.Name{entry.Information.Name},
			.Phases{Disassemble(entry.Specification.PhaseMask)},
			.Modes{Disassemble(entry.Specification.ModeMask)},
			.Order{entry.Specification.Order},
			.SystemData{SystemRegistry::SerializeSystem(entry.Instance, entry.Information.RuntimeId)}
		};

	};





	// --------------------------------------------------------------------------------------------------------

	SystemPipeline::Builder SystemScheduleSerializer::Deserializer(const SystemPipelineJson& systemPipelineJson) {
		
		SystemPipeline::Builder builder{};

		for (const auto systemJson : systemPipelineJson.Systems) {
		
			builder.AddSystem((SystemRegistration)systemJson);

		}

		return builder;

	};


	std::optional<SystemPipelineJson> SystemScheduleSerializer::BuildSystemPipelineJson(const SystemSchedule& schedule) {

		SystemPipelineJson pipelineJson;

		for (const SystemEntry& entry : schedule.GetSystems()) {
		
			pipelineJson.Systems.emplace_back(BuildSystemJson(entry));

		}

		return pipelineJson;

	};
	std::optional<SystemPipelineJson> SystemScheduleSerializer::BuildSystemPipelineJsonFrom(std::string_view jsonStr) {

		SystemPipelineJson pipelineJson;
		auto err = glz::read_json(pipelineJson, jsonStr);
		if (err) {
			AYIN_CORE_ERROR("Failed to parse system pipeline JSON: {}", glz::format_error(err, jsonStr));
			return std::nullopt;
		}

		return pipelineJson;

	};


}
