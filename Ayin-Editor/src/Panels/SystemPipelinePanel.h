#pragma once

#include <Ayin.h>

namespace Ayin {

	// System Pipeline 面板直接组织 Monitor / Editor 的交互；它只保存 World 的非拥有观察，并不参与 Apply。
	class SystemPipelinePanel {

	public:

		SystemPipelinePanel() = default;

		void SetContext(View<World> world);

		void OnImGuiRender();

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
		SystemTypeKey m_SelectedSystemType;
		std::string m_LastError;

	};

}
