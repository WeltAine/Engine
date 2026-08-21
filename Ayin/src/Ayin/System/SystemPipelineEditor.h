#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/System/SystemPipeline.h"
#include "Ayin/System/SystemSchedule.h"

#include <optional>

namespace Ayin {

	// Editor 结构编辑的临时会话。
	// Builder 保存结构草稿，Preview Schedule 只用于显示配置，不参与真实运行。
	class AYIN_API SystemPipelineEditor {

	public:

		SystemPipelineEditor() = default;

		bool Begin(const SystemSchedule& schedule);
		bool RebuildPreview();
		bool SyncPreviewConfiguration();
		std::optional<SystemPipeline> BuildPipeline();
		void Cancel();

		inline bool IsEditing() const { return m_IsEditing; };
		inline SystemPipeline::Builder& GetBuilder() { return m_Builder; };
		inline const SystemPipeline::Builder& GetBuilder() const { return m_Builder; };
		inline const SystemSchedule& GetPreviewSchedule() const { return m_PreviewSchedule; };
		inline SystemSchedule& GetPreviewSchedule() { return m_PreviewSchedule; };

		// 预览状态下允许显示 System 自己的配置面板，但不运行生命周期和 Update。
		inline void OnGui() { m_PreviewSchedule.OnGui(); };

	private:

		SystemPipeline::Builder m_Builder;
		SystemSchedule m_PreviewSchedule;
		bool m_IsEditing = false;

	};

};