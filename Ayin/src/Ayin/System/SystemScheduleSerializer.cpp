#include <AyinPch.h>

#include "Ayin/System/SystemScheduleSerializer.h"
#include "Ayin/System/SystemSchedule.h"

#include <unordered_set>

namespace Ayin {

	namespace {

		std::optional<SystemPipelineEntryDocument> BuildSystemPipelineEntryDocument(const SystemEntry& entry) {

			const auto configuration = SystemRegistry::SerializeConfiguration(
				*entry.GetInstance(), entry.Information.RuntimeId);
			if (!configuration) {
				AYIN_CORE_ERROR("Failed to serialize system '{}': {}", entry.Information.TypeKey, configuration.Error);
				return std::nullopt;
			}

			return SystemPipelineEntryDocument{
				.Type{entry.Information.TypeKey},
				.Phases{Disassemble(entry.Specification.PhaseMask)},
				.Modes{Disassemble(entry.Specification.ModeMask)},
				.Order{entry.Specification.Order},
				.Configuration{configuration.Json}
			};

		};


	};


	std::optional<SystemPipeline::Builder> SystemScheduleSerializer::Deserialize(
		const SystemPipelineDocument& document) {

		SystemPipeline::Builder builder{};
		std::unordered_set<SystemID> systemIds;

		for (const SystemPipelineEntryDocument& entry : document.Systems) {

			const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(entry.Type);
			if (descriptor == nullptr) {
				AYIN_CORE_ERROR("System Pipeline document refers to unknown System '{}'", entry.Type);
				return std::nullopt;
			}

			if (!systemIds.emplace(descriptor->RuntimeId).second) {
				AYIN_CORE_ERROR("System '{}' appears more than once in the Pipeline document", descriptor->TypeKey);
				return std::nullopt;
			}

			builder.AddSystem(SystemDefinition{
				.Type{descriptor->TypeKey},
				.Specification{
					.PhaseMask{Synthesis(entry.Phases)},
					.ModeMask{Synthesis(entry.Modes)},
					.Order{entry.Order}
				},
				.Configuration{.Json{entry.Configuration.str.empty()
					? SystemPipelineEntryDocument::NullConfiguration
					: entry.Configuration.str}}
			});
		}

		return builder;

	};


	std::optional<SystemPipelineDocument> SystemScheduleSerializer::Serialize(const SystemSchedule& schedule) {

		SystemPipelineDocument document;
		document.Systems.reserve(schedule.GetSystems().size());

		for (const SystemEntry& entry : schedule.GetSystems()) {
			const auto documentEntry = BuildSystemPipelineEntryDocument(entry);
			if (!documentEntry)
				return std::nullopt;

			document.Systems.emplace_back(std::move(*documentEntry));
		}

		return document;

	};

	std::optional<SystemPipelineDocument> SystemScheduleSerializer::Parse(const std::string_view jsonStr) {

		SystemPipelineDocument document;
		auto err = glz::read_json(document, jsonStr);
		if (err) {
			AYIN_CORE_ERROR("Failed to parse System Pipeline JSON: {}", glz::format_error(err, jsonStr));
			return std::nullopt;
		}

		return document;

	};


}