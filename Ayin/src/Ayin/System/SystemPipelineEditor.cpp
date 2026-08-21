#include <AyinPch.h>

#include "Ayin/System/SystemPipelineEditor.h"
#include "Ayin/System/SystemScheduleSerializer.h"

namespace Ayin {

	bool SystemPipelineEditor::Begin(const SystemSchedule& schedule) {

		const auto document = SystemScheduleSerializer::BuildSystemPipelineJson(schedule);
		if (!document)
			return false;

		m_Builder = SystemScheduleSerializer::Deserializer(*document);
		if (!RebuildPreview()) {
			Cancel();
			return false;
		}

		m_IsEditing = true;
		return true;

	};


	bool SystemPipelineEditor::RebuildPreview() {

		SystemPipeline pipeline = m_Builder.Build();
		if (!pipeline.IsValid())
			return false;

		SystemSchedule preview = pipeline.CreateSchedule();
		if (!preview.IsBuilt())
			return false;

		m_PreviewSchedule = std::move(preview);
		return true;

	};


	bool SystemPipelineEditor::SyncPreviewConfiguration() {

		const auto document = SystemScheduleSerializer::BuildSystemPipelineJson(m_PreviewSchedule);
		if (!document)
			return false;

		for (const SystemJson& system : document->Systems) {
			m_Builder.SetSystemConfiguration(
				SystemRegistry::GetSystemDescriptor(system.Name)->RuntimeId,
				SystemConfiguration{ .Json{ system.SystemData.str } });
		}

		return true;

	};


	std::optional<SystemPipeline> SystemPipelineEditor::BuildPipeline() {

		if (!m_IsEditing || !SyncPreviewConfiguration())
			return std::nullopt;

		SystemPipeline pipeline = m_Builder.Build();
		if (!pipeline.IsValid())
			return std::nullopt;

		return pipeline;

	};


	void SystemPipelineEditor::Cancel() {

		m_Builder = {};
		m_PreviewSchedule = {};
		m_IsEditing = false;

	};

};