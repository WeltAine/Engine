#pragma once

#include <Ayin.h>

#include <optional>
#include <string_view>

namespace Ayin {

	// System Pipeline 面板直接组织 Monitor / Editor 的交互；它只保存 World 的非拥有观察，并只提交 Apply 请求。
	class SystemPipelinePanel {

	public:

		SystemPipelinePanel() = default;

		void SetContext(View<World> world);

		void OnImGuiRender();

		// Apply 请求由 EditorLayer 在下一帧 Update 前处理，避免在当前 Schedule 遍历时替换 World。
		std::optional<SystemPipeline> TakePendingPipeline();
		void CompleteApply();
		void RejectApply(std::string_view error);

		// 临时 World 已被 Apply 结束时，保留 Builder / Preview 草稿并重新观察 EditorWorld。
		void KeepEditingAfterApplyFailure(View<World> world);

	private:

		void DrawToolbar();

		void DrawRuntimeList(const SystemSchedule& schedule);
		void DrawBuilderList();

		void DrawRuntimeProperties();
		void DrawPreviewProperties();
		void DrawAddSystemPopup();

		bool BeginEditing();
		bool RebuildPreview();
		void CancelEditing();

		static std::string GetPhaseText(SystemPhase phaseMask);
		static std::string GetModeText(SceneMode modeMask);

	private:

		View<World> m_World;
		SystemPipelineEditor m_PipelineEditor;
		std::optional<SystemPipeline> m_PendingPipeline;
		SystemTypeKey m_SelectedSystemType;
		std::string m_LastError;

	};

}
