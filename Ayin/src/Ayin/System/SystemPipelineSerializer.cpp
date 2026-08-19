#include <AyinPch.h>

#include "Ayin/System/SystemPipelineSerializer.h"

namespace Ayin {
	
	std::optional<SystemPipelineJson> SystemPipelineSerializer::BuildSystemPipelineJson() {

		if(!m_SystemPipeline)
			return std::nullopt;

		SystemPipelineJson pipelineJson;

		for (SystemRegistration& registration : m_SystemPipeline->m_Registrations) {
		
			pipelineJson.Systems.emplace_back(
			
				SystemJson{
					.Name{registration.Information.Name},
					.Phases{Disassemble(registration.Specification.PhaseMask)},
					.Modes{Disassemble(registration.Specification.ModeMask)},
					.Order{registration.Specification.Order},
					.SystemData{registration.SystemData}
				}

			);

		}

		return pipelineJson;

	};
	std::optional<SystemPipelineJson> SystemPipelineSerializer::BuildSystemPipelineJsonFrom(std::string_view jsonStr) {

		SystemPipelineJson pipelineJson;
		auto err = glz::read_json(pipelineJson, jsonStr);
		if (err) {
			AYIN_CORE_ERROR("Failed to parse scene JSON: {}", glz::format_error(err, jsonStr));
			return std::nullopt;
		}

		return pipelineJson;

	};


}
