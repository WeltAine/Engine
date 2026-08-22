#include <AyinPch.h>

#include <imgui.h>

#include <algorithm>
#include <cctype>

#include "Panels/SystemPipelinePanel.h"


namespace Ayin {

	void SystemPipelinePanel::SetContext(const View<World> world) {

		if (m_World.Get() == world.Get())
			return;

		// 切换活动 World 时不能继续拿旧 World 的 Builder / Preview 草稿编辑。
		CancelEditing();
		m_World = world;
		m_SelectedSystemType.clear();
		m_LastError.clear();

	};


	std::optional<SystemPipeline> SystemPipelinePanel::TakePendingPipeline() {

		if (!m_PendingPipeline)
			return std::nullopt;

		SystemPipeline pipeline = std::move(*m_PendingPipeline);
		m_PendingPipeline.reset();
		return pipeline;

	};


	void SystemPipelinePanel::CompleteApply() {

		CancelEditing();

	};


	void SystemPipelinePanel::RejectApply(const std::string_view error) {

		m_LastError = error;

	};


	void SystemPipelinePanel::KeepEditingAfterApplyFailure(const View<World> world) {

		m_World = world;

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

		DrawToolbar();

		const bool applyPending = m_PendingPipeline.has_value();
		if (applyPending)
			ImGui::BeginDisabled();

		if (!m_LastError.empty()) {
			ImGui::TextColored(ImVec4{ 0.95f, 0.35f, 0.35f, 1.0f }, "%s", m_LastError.c_str());
			ImGui::Separator();
		}

		// 筛选和分栏只改变面板的观察方式，不改变选中 System 或任何 Pipeline 结构。
		DrawSystemFilter();
		ImGui::Separator();

		constexpr float minimumListWidth = 220.0f;
		constexpr float minimumPropertiesWidth = 260.0f;
		constexpr float splitterWidth = 6.0f;

		const ImVec2 availableSize = ImGui::GetContentRegionAvail();
		const float maximumListWidth = std::max(
			minimumListWidth,
			availableSize.x - splitterWidth - minimumPropertiesWidth);
		if (m_ListWidth <= 0.0f)
			m_ListWidth = std::max(minimumListWidth, availableSize.x * 0.42f);
		m_ListWidth = std::clamp(m_ListWidth, minimumListWidth, maximumListWidth);

		ImGui::BeginChild("##SystemList", ImVec2{ m_ListWidth, availableSize.y }, ImGuiChildFlags_Borders);
		if (m_PipelineEditor.IsEditing())
			DrawBuilderList();
		else
			DrawRuntimeList(schedule);
		ImGui::EndChild();

		ImGui::SameLine(0.0f, 0.0f);
		DrawSplitter(availableSize.y);
		ImGui::SameLine(0.0f, 0.0f);

		ImGui::BeginChild("##SystemProperties", ImVec2{ 0.0f, availableSize.y }, ImGuiChildFlags_Borders);
		if (m_PipelineEditor.IsEditing())
			DrawPreviewProperties();
		else
			DrawRuntimeProperties();
		ImGui::EndChild();

		if (applyPending)
			ImGui::EndDisabled();

		ImGui::End();

	};


	void SystemPipelinePanel::DrawToolbar() {

		if (!m_PipelineEditor.IsEditing()) {
			if (ImGui::Button("Editor"))
				BeginEditing();

			ImGui::SameLine();
			ImGui::TextDisabled("Monitor");
			return;
		}

		ImGui::TextDisabled("Editor Preview");
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			CancelEditing();

		ImGui::SameLine();
		if (m_PendingPipeline) {
			ImGui::BeginDisabled();
			ImGui::Button("Apply");
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::TextDisabled("Apply is pending");
			return;
		}

		if (!ImGui::Button("Apply"))
			return;

		const std::optional<SystemPipeline> pipeline = m_PipelineEditor.BuildPipeline();
		if (!pipeline) {
			m_LastError = "Failed to build the candidate System Pipeline";
			return;
		}

		m_PendingPipeline = std::move(*pipeline);
		m_LastError.clear();

	};


	void SystemPipelinePanel::DrawSystemFilter() {

		ImGui::SetNextItemWidth(-72.0f);
		ImGui::InputTextWithHint(
			"##SystemFilter",
			"Search System...",
			m_SystemFilter.data(),
			m_SystemFilter.size());

		ImGui::SameLine();
		if (ImGui::Button("Clear") && m_SystemFilter.front() != '\0')
			m_SystemFilter.fill('\0');

	};


	void SystemPipelinePanel::DrawSplitter(const float height) {

		// Splitter 只保存当前面板宽度；下一帧会统一处理窗口缩放后的范围收束。
		ImGui::InvisibleButton("##SystemListSplitter", ImVec2{ 6.0f, height });
		if (ImGui::IsItemActive())
			m_ListWidth += ImGui::GetIO().MouseDelta.x;

		if (ImGui::IsItemHovered() || ImGui::IsItemActive())
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

	};


	void SystemPipelinePanel::DrawRuntimeList(const SystemSchedule& schedule) {

		const std::vector<SystemEntry>& systems = schedule.GetSystems();

		if (systems.empty()) {
			ImGui::TextUnformatted("The current Schedule contains no System");
			return;
		}

		std::size_t visibleSystemCount = 0;

		constexpr ImGuiTableFlags tableFlags =
			ImGuiTableFlags_BordersInnerV |
			ImGuiTableFlags_RowBg |
			ImGuiTableFlags_SizingStretchProp |
			ImGuiTableFlags_ScrollY;

		if (!ImGui::BeginTable("##RuntimeSystems", 4, tableFlags))
			return;

		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 2.5f);
		ImGui::TableSetupColumn("Phase", ImGuiTableColumnFlags_WidthStretch, 1.8f);
		ImGui::TableSetupColumn("Mode", ImGuiTableColumnFlags_WidthStretch, 1.5f);
		ImGui::TableSetupColumn("Order", ImGuiTableColumnFlags_WidthFixed, 56.0f);
		ImGui::TableHeadersRow();

		for (const SystemEntry& entry : systems) {

			const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(entry.Information.RuntimeId);
			const std::string_view displayName = descriptor == nullptr
				? std::string_view{}
				: std::string_view{ descriptor->DisplayName };
			if (!PassesSystemFilter(entry.Information.TypeKey, displayName))
				continue;

			++visibleSystemCount;
			const bool selected = m_SelectedSystemType == entry.Information.TypeKey;

			ImGui::PushID(static_cast<int>(entry.Information.RuntimeId));

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (ImGui::Selectable(entry.Information.TypeKey.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns))
				m_SelectedSystemType = entry.Information.TypeKey;

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

		if (visibleSystemCount == 0)
			ImGui::TextDisabled("No System matches the current filter");

	};


	void SystemPipelinePanel::DrawBuilderList() {

		SystemPipeline::Builder& builder = m_PipelineEditor.GetBuilder();
		const SystemPipeline::Builder::DefinitionList& definitions = builder.GetDefinitions();

		if (ImGui::Button("Add System"))
			ImGui::OpenPopup("AddSystem");

		DrawAddSystemPopup();
		ImGui::Separator();

		if (definitions.empty()) {
			ImGui::TextUnformatted("The Builder contains no System definition");
			return;
		}

		std::size_t visibleDefinitionCount = 0;

		constexpr ImGuiTableFlags tableFlags =
			ImGuiTableFlags_BordersInnerV |
			ImGuiTableFlags_RowBg |
			ImGuiTableFlags_SizingStretchProp |
			ImGuiTableFlags_ScrollY;

		if (!ImGui::BeginTable("##BuilderDefinitions", 4, tableFlags))
			return;

		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 2.5f);
		ImGui::TableSetupColumn("Phase", ImGuiTableColumnFlags_WidthStretch, 1.8f);
		ImGui::TableSetupColumn("Mode", ImGuiTableColumnFlags_WidthStretch, 1.5f);
		ImGui::TableSetupColumn("Order", ImGuiTableColumnFlags_WidthFixed, 56.0f);
		ImGui::TableHeadersRow();

		for (const SystemDefinition& definition : definitions) {

			const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(definition.Type);
			const std::string_view displayName = descriptor == nullptr
				? std::string_view{}
				: std::string_view{ descriptor->DisplayName };
			if (!PassesSystemFilter(definition.Type, displayName))
				continue;

			++visibleDefinitionCount;
			const bool selected = m_SelectedSystemType == definition.Type;

			ImGui::PushID(definition.Type.c_str());

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (ImGui::Selectable(definition.Type.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns))
				m_SelectedSystemType = definition.Type;

			ImGui::TableSetColumnIndex(1);
			const std::string phaseText = GetPhaseText(definition.Specification.PhaseMask);
			ImGui::TextUnformatted(phaseText.c_str());

			ImGui::TableSetColumnIndex(2);
			const std::string modeText = GetModeText(definition.Specification.ModeMask);
			ImGui::TextUnformatted(modeText.c_str());

			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%d", definition.Specification.Order);

			ImGui::PopID();

		}

		ImGui::EndTable();

		if (visibleDefinitionCount == 0)
			ImGui::TextDisabled("No System matches the current filter");

	};


	void SystemPipelinePanel::DrawRuntimeProperties() {

		if (m_SelectedSystemType.empty()) {
			ImGui::TextUnformatted("Select a System to inspect its runtime instance");
			return;
		}

		const SystemEntry* entry = m_World->GetSystemSchedule().FindSystemEntry(m_SelectedSystemType);
		ISystem* system = m_World->FindSystemInstance(m_SelectedSystemType);

		if (entry == nullptr || system == nullptr) {
			// 当前 World 替换后，旧 TypeKey 已经不再有对应实例。
			m_SelectedSystemType.clear();
			ImGui::TextUnformatted("The selected System is no longer available");
			return;
		}

		ImGui::TextUnformatted(entry->Information.TypeKey.c_str());
		ImGui::Text("Order: %d", entry->Specification.Order);
		ImGui::Separator();

		// System 自己决定如何绘制和修改运行时参数；面板不包装 GUI 调用，也不修改 Schedule 拓扑。
		system->OnEditorGui();

	};


	void SystemPipelinePanel::DrawPreviewProperties() {

		if (m_SelectedSystemType.empty()) {
			ImGui::TextUnformatted("Select a Builder definition to edit its Preview System");
			return;
		}

		SystemPipeline::Builder& builder = m_PipelineEditor.GetBuilder();
		const SystemDefinition* selectedDefinition = nullptr;
		for (const SystemDefinition& definition : builder.GetDefinitions()) {
			if (definition.Type == m_SelectedSystemType) {
				selectedDefinition = &definition;
				break;
			}
		}

		if (selectedDefinition == nullptr) {
			m_SelectedSystemType.clear();
			ImGui::TextUnformatted("The selected Builder definition is no longer available");
			return;
		}

		const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(selectedDefinition->Type);
		if (descriptor == nullptr) {
			m_LastError = "The selected System is not registered";
			return;
		}

		ImGui::TextUnformatted(descriptor->DisplayName.c_str());
		ImGui::TextDisabled("%s", selectedDefinition->Type.c_str());
		ImGui::Separator();

		SystemSpecification specification = selectedDefinition->Specification;
		bool specificationChanged = false;

		bool preUpdate = static_cast<bool>(specification.PhaseMask & SystemPhase::PreUpdate);
		if (ImGui::Checkbox("PreUpdate", &preUpdate)) {
			if (preUpdate)
				specification.PhaseMask |= SystemPhase::PreUpdate;
			else
				specification.PhaseMask &= ~SystemPhase::PreUpdate;
			specificationChanged = true;
		}

		bool update = static_cast<bool>(specification.PhaseMask & SystemPhase::Update);
		if (ImGui::Checkbox("Update", &update)) {
			if (update)
				specification.PhaseMask |= SystemPhase::Update;
			else
				specification.PhaseMask &= ~SystemPhase::Update;
			specificationChanged = true;
		}

		bool postUpdate = static_cast<bool>(specification.PhaseMask & SystemPhase::PostUpdate);
		if (ImGui::Checkbox("PostUpdate", &postUpdate)) {
			if (postUpdate)
				specification.PhaseMask |= SystemPhase::PostUpdate;
			else
				specification.PhaseMask &= ~SystemPhase::PostUpdate;
			specificationChanged = true;
		}

		bool presentation = static_cast<bool>(specification.PhaseMask & SystemPhase::Presentation);
		if (ImGui::Checkbox("Presentation", &presentation)) {
			if (presentation)
				specification.PhaseMask |= SystemPhase::Presentation;
			else
				specification.PhaseMask &= ~SystemPhase::Presentation;
			specificationChanged = true;
		}

		ImGui::Separator();

		bool editor = Contains(specification.ModeMask, SceneMode::Editor);
		if (ImGui::Checkbox("Editor", &editor)) {
			if (editor)
				specification.ModeMask |= SceneMode::Editor;
			else
				specification.ModeMask &= ~SceneMode::Editor;
			specificationChanged = true;
		}

		bool simulation = Contains(specification.ModeMask, SceneMode::Simulation);
		if (ImGui::Checkbox("Simulation", &simulation)) {
			if (simulation)
				specification.ModeMask |= SceneMode::Simulation;
			else
				specification.ModeMask &= ~SceneMode::Simulation;
			specificationChanged = true;
		}

		bool runtime = Contains(specification.ModeMask, SceneMode::Runtime);
		if (ImGui::Checkbox("Runtime", &runtime)) {
			if (runtime)
				specification.ModeMask |= SceneMode::Runtime;
			else
				specification.ModeMask &= ~SceneMode::Runtime;
			specificationChanged = true;
		}

		ImGui::Separator();

		if (ImGui::InputInt("Order", &specification.Order))
			specificationChanged = true;

		if (specificationChanged) {
			builder.SetSystemSpecification(descriptor->RuntimeId, specification);
			if (!RebuildPreview())
				return;
		}

		if (ImGui::Button("Remove System")) {
			builder.RemoveSystem(descriptor->RuntimeId);
			m_SelectedSystemType.clear();
			RebuildPreview();
			return;
		}

		ImGui::Separator();
		ImGui::TextUnformatted("Preview Parameters");

		ISystem* previewSystem = m_PipelineEditor.GetPreviewSchedule().FindSystemInstance(m_SelectedSystemType);
		if (previewSystem == nullptr) {
			m_LastError = "The selected Preview System could not be created";
			return;
		}

		// Preview 没有进入任何运行生命周期；这里的参数修改只停留在编辑会话中。
		previewSystem->OnEditorGui();

	};


	void SystemPipelinePanel::DrawAddSystemPopup() {

		if (!ImGui::BeginPopup("AddSystem"))
			return;

		SystemTypeKey selectedType;
		const SystemPipeline::Builder& builder = m_PipelineEditor.GetBuilder();

		for (const SystemDescriptor& descriptor : SystemRegistry::GetAllSystemDescriptors()) {

			const bool alreadyAdded = builder.ContainSystem(descriptor.RuntimeId);

			ImGui::PushID(static_cast<int>(descriptor.RuntimeId));
			ImGui::BeginDisabled(alreadyAdded);
			if (ImGui::Selectable(descriptor.DisplayName.c_str()))
				selectedType = descriptor.TypeKey;
			ImGui::EndDisabled();
			ImGui::PopID();

		}

		if (!selectedType.empty()) {
			const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(selectedType);
			if (descriptor != nullptr) {
				m_PipelineEditor.GetBuilder().AddSystem(SystemDefinition{
					.Type{descriptor->TypeKey},
					.Specification{descriptor->DefaultSpecification}
				});

				if (RebuildPreview())
					m_SelectedSystemType = descriptor->TypeKey;
			}

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();

	};


	bool SystemPipelinePanel::BeginEditing() {

		m_LastError.clear();
		m_SelectedSystemType.clear();

		if (m_PipelineEditor.Begin(m_World->GetSystemSchedule()))
			return true;

		m_LastError = "Failed to create the Builder or Preview Schedule";
		return false;

	};


	bool SystemPipelinePanel::RebuildPreview() {

		// SystemPipelineEditor 会在重建前同步 Preview 配置，面板只负责触发结构更新。
		if (!m_PipelineEditor.RebuildPreview()) {
			m_LastError = "Failed to rebuild the Preview Schedule";
			return false;
		}

		m_LastError.clear();
		return true;

	};


	void SystemPipelinePanel::CancelEditing() {

		m_PipelineEditor.Cancel();
		m_PendingPipeline.reset();
		m_SelectedSystemType.clear();
		m_LastError.clear();

	};


	bool SystemPipelinePanel::PassesSystemFilter(
		const std::string_view typeKey,
		const std::string_view displayName) const {

		// 搜索不区分大小写，同时匹配稳定 TypeKey 和可读 DisplayName。
		const std::string_view filter{ m_SystemFilter.data() };
		if (filter.empty())
			return true;

		auto containsIgnoringCase = [filter](const std::string_view value) {

			const auto result = std::ranges::search(
				value,
				filter,
				[](const char left, const char right) {
					return std::tolower(static_cast<unsigned char>(left)) ==
						std::tolower(static_cast<unsigned char>(right));
				});

			return !result.empty();

		};

		return containsIgnoringCase(typeKey) || containsIgnoringCase(displayName);

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
