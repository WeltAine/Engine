#include <AyinPch.h>

#include <imgui.h>

#include "Panels/SystemPipelinePanel.h"


namespace Ayin {

	void SystemPipelinePanel::SetContext(const View<World> world) {

		if (m_World.Get() == world.Get())
			return;

		m_World = world;
		m_SelectedSystem.reset();

	};


	void SystemPipelinePanel::OnImGuiRender() {

		ImGui::Begin("System Pipeline");

		if (!m_World) {
			ImGui::TextUnformatted("No active World");
			ImGui::End();
			return;
		}

		const SystemSchedule& schedule = m_World->GetSystemSchedule();

		if (!schedule.IsBuilt()) {
			ImGui::TextUnformatted("The active World has no built System Schedule");
			ImGui::End();
			return;
		}

		const float listWidth = std::max(240.0f, ImGui::GetContentRegionAvail().x * 0.42f);

		ImGui::BeginChild("##SystemList", ImVec2{ listWidth, 0.0f }, ImGuiChildFlags_Borders);
		DrawSystemList(schedule);
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("##SystemProperties", ImVec2{ 0.0f, 0.0f }, ImGuiChildFlags_Borders);
		DrawSystemProperties();
		ImGui::EndChild();

		ImGui::End();

	};


	void SystemPipelinePanel::DrawSystemList(const SystemSchedule& schedule) {

		const std::vector<SystemEntry>& systems = schedule.GetSystems();

		if (systems.empty()) {
			ImGui::TextUnformatted("The current Schedule contains no System");
			return;
		}

		constexpr ImGuiTableFlags tableFlags =
			ImGuiTableFlags_BordersInnerV |
			ImGuiTableFlags_RowBg |
			ImGuiTableFlags_SizingStretchProp |
			ImGuiTableFlags_ScrollY;

		if (!ImGui::BeginTable("##Systems", 4, tableFlags))
			return;

		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 2.5f);
		ImGui::TableSetupColumn("Phase", ImGuiTableColumnFlags_WidthStretch, 1.8f);
		ImGui::TableSetupColumn("Mode", ImGuiTableColumnFlags_WidthStretch, 1.5f);
		ImGui::TableSetupColumn("Order", ImGuiTableColumnFlags_WidthFixed, 56.0f);
		ImGui::TableHeadersRow();

		for (const SystemEntry& entry : systems) {

			const bool selected = m_SelectedSystem && *m_SelectedSystem == entry.Information.RuntimeId;

			ImGui::PushID(static_cast<int>(entry.Information.RuntimeId));

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (ImGui::Selectable(entry.Information.TypeKey.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns))
				m_SelectedSystem = entry.Information.RuntimeId;

			ImGui::TableSetColumnIndex(1);
			const std::string phaseText = GetPhaseText(entry.Specification.PhaseMask);
			ImGui::TextUnformatted(phaseText.c_str());

			ImGui::TableSetColumnIndex(2);
			const std::string modeText = GetModeText(entry.Specification.ModeMask);
			ImGui::TextUnformatted(modeText.c_str());

			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%d", entry.Specification.Order);

			ImGui::PopID();

		}

		ImGui::EndTable();

	};


	void SystemPipelinePanel::DrawSystemProperties() {

		if (!m_SelectedSystem) {
			ImGui::TextUnformatted("Select a System to inspect its runtime instance");
			return;
		}

		const SystemEntry* entry = m_World->GetSystemSchedule().FindSystemEntry(*m_SelectedSystem);
		ISystem* system = m_World->FindSystemInstance(*m_SelectedSystem);

		if (entry == nullptr || system == nullptr) {
			// 当前 World 替换后，旧的 RuntimeId 已经不再有对应实例。
			m_SelectedSystem.reset();
			ImGui::TextUnformatted("The selected System is no longer available");
			return;
		}

		ImGui::TextUnformatted(entry->Information.TypeKey.c_str());
		ImGui::Text("Order: %d", entry->Specification.Order);
		ImGui::Separator();

		// System 自己决定如何绘制和修改运行时参数；面板不包装 GUI 调用，也不修改 Schedule 拓扑。
		system->OnEditorGui();

	};


	std::string SystemPipelinePanel::GetPhaseText(const SystemPhase phaseMask) {

		std::string result;

		auto appendPhase = [&result, phaseMask](const SystemPhase phase, const char* name) {
			if (!static_cast<bool>(phaseMask & phase))
				return;

			if (!result.empty())
				result += " | ";

			result += name;
		};

		appendPhase(SystemPhase::PreUpdate, "PreUpdate");
		appendPhase(SystemPhase::Update, "Update");
		appendPhase(SystemPhase::PostUpdate, "PostUpdate");
		appendPhase(SystemPhase::Presentation, "Presentation");

		return result.empty() ? "None" : result;

	};


	std::string SystemPipelinePanel::GetModeText(const SceneMode modeMask) {

		std::string result;

		auto appendMode = [&result, modeMask](const SceneMode mode, const char* name) {
			if (!Contains(modeMask, mode))
				return;

			if (!result.empty())
				result += " | ";

			result += name;
		};

		appendMode(SceneMode::Editor, "Editor");
		appendMode(SceneMode::Simulation, "Simulation");
		appendMode(SceneMode::Runtime, "Runtime");

		return result.empty() ? "None" : result;

	};

}
