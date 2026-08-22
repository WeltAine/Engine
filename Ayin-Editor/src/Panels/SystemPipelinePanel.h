#pragma once

#include <Ayin.h>

#include <optional>

namespace Ayin {

	// System Pipeline 面板只观察当前运行中的 World；结构编辑、Preview 和 Apply 由后续阶段实现。
	class SystemPipelinePanel {

	public:

		SystemPipelinePanel() = default;

		void SetContext(View<World> world);

		void OnImGuiRender();

	private:

		void DrawSystemList(const SystemSchedule& schedule);
		void DrawSystemProperties();

		static std::string GetPhaseText(SystemPhase phaseMask);
		static std::string GetModeText(SceneMode modeMask);

	private:

		View<World> m_World;
		std::optional<SystemID> m_SelectedSystem;

	};

}
