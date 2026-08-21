#include <AyinPch.h>

#include "Ayin/System/SystemScheduleSerializer.h"
#include "Ayin/System/SystemSchedule.h"

namespace Ayin {

	// ---------------------------------------------- 辅助方法 -------------------------------------------------------

	SystemJson BuildSystemJson(const SystemEntry& entry) {

		return SystemJson{
			.Name{entry.Information.TypeKey},
			.Phases{Disassemble(entry.Specification.PhaseMask)},
			.Modes{Disassemble(entry.Specification.ModeMask)},
			.Order{entry.Specification.Order},
			.SystemData{[&entry]() -> ::glz::raw_json {
				const auto result = SystemRegistry::SerializeConfiguration(*entry.Instance, entry.Information.RuntimeId);
				if (!result) {
					AYIN_CORE_ERROR("Failed to serialize system '{}': {}", entry.Information.TypeKey, result.Error);
					return SystemJson::NullSystemData;
				}

				return result.Json;
			}()}
		};

	};


	// --------------------------------------------------------------------------------------------------------

	SystemPipeline::Builder SystemScheduleSerializer::Deserializer(const SystemPipelineJson& systemPipelineJson) {

		SystemPipeline::Builder builder{};

		for (const SystemJson& systemJson : systemPipelineJson.Systems) {
			builder.AddSystem(SystemDefinition{
				.Type{systemJson.Name},
				.Specification{
					.PhaseMask{Synthesis(systemJson.Phases)},
					.ModeMask{Synthesis(systemJson.Modes)},
					.Order{systemJson.Order}
				},
				.Configuration{.Json{systemJson.SystemData.str}}
			});
		}

		return builder;

	};


	std::optional<SystemPipelineJson> SystemScheduleSerializer::BuildSystemPipelineJson(const SystemSchedule& schedule) {

		SystemPipelineJson pipelineJson;

		for (const SystemEntry& entry : schedule.GetSystems())
			pipelineJson.Systems.emplace_back(BuildSystemJson(entry));

		return pipelineJson;

	};

	std::optional<SystemPipelineJson> SystemScheduleSerializer::BuildSystemPipelineJsonFrom(const std::string_view jsonStr) {

		SystemPipelineJson pipelineJson;
		auto err = glz::read_json(pipelineJson, jsonStr);
		if (err) {
			AYIN_CORE_ERROR("Failed to parse system pipeline JSON: {}", glz::format_error(err, jsonStr));
			return std::nullopt;
		}

		return pipelineJson;

	};


}
