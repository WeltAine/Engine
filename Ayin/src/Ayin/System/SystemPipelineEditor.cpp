#include <AyinPch.h>

#include "Ayin/System/SystemPipelineEditor.h"
#include "Ayin/System/SystemScheduleSerializer.h"

namespace Ayin {

	bool SystemPipelineEditor::Begin(const SystemSchedule& schedule) {

		const auto document = SystemScheduleSerializer::Serialize(schedule);
		if (!document)
			return false;

		const auto builder = SystemScheduleSerializer::Deserialize(*document);
		if (!builder)
			return false;

		m_Builder = *builder;
		m_IsEditing = true;
		if (!RebuildPreview()) {
			Cancel();
			return false;
		}

		return true;

	};


	bool SystemPipelineEditor::RebuildPreview() {

		// 结构重建前先保存 Preview 参数，避免 Builder 的定义修改吞掉尚未 Apply 的配置草稿。
		if (m_IsEditing && !SyncPreviewConfiguration())
			return false;

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

		const auto document = SystemScheduleSerializer::Serialize(m_PreviewSchedule);
		if (!document)
			return false;

		for (const SystemPipelineEntryDocument& entry : document->Systems) {
			const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(entry.Type);
			if (descriptor == nullptr) {
				AYIN_CORE_ERROR("Preview refers to unknown System '{}'", entry.Type);
				return false;
			}

			m_Builder.SetSystemConfiguration(
				descriptor->RuntimeId,
				SystemConfiguration{ .Json{ entry.Configuration.str } });
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