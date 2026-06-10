#include <AyinPch.h>

#include <imgui.h>

#include "Panels/PropertiesPanel.h"


namespace Ayin {

	void PropertiesPanel::OnImGuiRender() {

		ImGui::Begin("Properties");

		if (m_SelectedEntity) {

			auto&& descriptors = ComponentRegistry::GetAllComponentDescriptors();

			ImGui::PushID(static_cast<uint32_t>(m_SelectedEntity));

			for (size_t i = 0; i < descriptors.size(); ++i) {
				auto& desc = descriptors[i];
				if (IsInternalComponent(desc.id)) {
					continue;
				}

				if (m_SelectedEntity.HasComponent(desc.id)) {
					if (desc.id == entt::type_hash<NativeScriptComponent>::value()) {
						DrawNativeScriptComponentHeader(m_SelectedEntity, false, desc.displayName.c_str());
					} else {
						DrawComponentHeader(desc);
					}

					ImGui::Separator();
				}
			}

			DrawAttachedNativeScripts();
			DrawInternalComponents();

			ImGui::Spacing();
			ImGui::Spacing();


			ImGui::PopID();

			if (ImGui::Button("Add Component", ImVec2(-1, 0))) {
				ImGui::OpenPopup("AddComponentSearch");//打开对应ID得弹窗
			}

			DrawAddComponentSearch();

		}

		ImGui::End();

	}

	void PropertiesPanel::DrawNativeScriptComponentHeader(Entity& entity, bool destroyEntityOnRemove, const char* title) {

		bool isOpen = false;
		bool removed = false;
		float lineHeight = ImGui::GetFrameHeight();

		ImGui::PushID(static_cast<uint32_t>(entity));
		ImGui::PushID(title);

		if (ImGui::BeginTable("##row", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_BordersInnerV)) {

			ImGui::TableSetupColumn("##left", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("##right", ImGuiTableColumnFlags_WidthFixed, lineHeight + 4.0f);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			isOpen = ImGui::TreeNodeEx(title, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen);

			ImGui::TableNextColumn();
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
			if (ImGui::Button("X", { lineHeight, lineHeight })) {
				if (destroyEntityOnRemove && m_Scene) {
					m_Scene->DestroyEntity(entity);
				} else if (entity.HasComponents<NativeScriptComponent>()) {
					entity.RemoveComponents<NativeScriptComponent>();
				}
				removed = true;
				isOpen = false;
			}
			ImGui::PopStyleColor();

			ImGui::EndTable();

		}

		if (!removed && isOpen && entity.HasComponents<NativeScriptComponent>()) {
			ImGui::Indent();
			DrawNativeScriptComponent(entity);
			ImGui::Unindent();
		}

		ImGui::PopID();
		ImGui::PopID();

	}

	void PropertiesPanel::DrawNativeScriptComponent(Entity& entity) {

		NativeScriptComponent& nsc = entity.GetComponents<NativeScriptComponent>();
		const char* currentScriptName = nsc.HasScript() ? nsc.ScriptName.c_str() : "None";

		ImGui::Text("Script: %s", currentScriptName);

		const char* buttonLabel = nsc.HasScript() ? "Switch Script" : "Add Script";
		if (ImGui::Button(buttonLabel)) {
			ImGui::OpenPopup("ScriptPicker");
		}

		ImGui::SameLine();
		if (ImGui::Button("Remove")) {
			if (entity.HasComponents<AttachmentComponent>() && m_Scene) {
				m_Scene->DestroyEntity(entity);
			} else {
				nsc.Unbind();
			}
			return;
		}

		if (ImGui::BeginPopup("ScriptPicker")) {
			auto scriptNames = ScriptRegistry::GetAvailableScriptNames();
			if (scriptNames.empty()) {
				ImGui::TextDisabled("No scripts registered");
			}

			for (const std::string& scriptName : scriptNames) {
				bool selected = nsc.ScriptName == scriptName;
				if (ImGui::Selectable(scriptName.c_str(), selected)) {
					nsc.Unbind();
					ScriptRegistry::BindScriptByScriptName(nsc, scriptName);
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

		if (nsc.ScriptableInstance != nullptr) {
			ImGui::Spacing();
			ImGui::SeparatorText("Script UI");
			nsc.ScriptableInstance->OnGui();
		}

	}

	void PropertiesPanel::DrawComponentHeader(const ComponentDescriptor& desc) {

		bool isOpen = false;

		float lineHeight = ImGui::GetFrameHeight();

		ImGui::PushID(desc.id);
		// 返回值的bool代表表格可见，且可渲染。
		if (ImGui::BeginTable("##row", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_BordersInnerV)) {//这个flag是什么意思？禁止在表格主体区域绘制边框

			ImGui::TableSetupColumn("##left", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("##right", ImGuiTableColumnFlags_WidthFixed, lineHeight + 4.0f);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			//组件名称绘制
			isOpen = ImGui::TreeNodeEx(desc.displayName.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen);

			//按钮绘制
			ImGui::TableNextColumn();
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
			if (ImGui::Button("X", { lineHeight, lineHeight })) {
				m_SelectedEntity.RemoveComponent(desc.id);
				isOpen = false;
			}
			ImGui::PopStyleColor();

			ImGui::EndTable();

		}

		if (isOpen) {
			ImGui::Indent();
			desc.onGUI(m_SelectedEntity);
			ImGui::Unindent();
		}

		ImGui::PopID();

	}


	void PropertiesPanel::DrawAddComponentSearch() {

		ImGui::SetNextWindowPos(
			ImGui::GetMainViewport()->GetCenter(),  // 目标：主视口正中心
			ImGuiCond_Appearing,                     // 条件：只在弹窗首次出现时设置位置
			ImVec2(0.5f, 0.5f)                      // 轴心：(0.5,0.5) = 以窗口自身中心对齐目标点
		);

		//BeginPopup 即使返回 false，内部也走了 Begin() → 立即 End() 的路径。这个 Begin() 里会消费 NextWindowData
		if (!ImGui::BeginPopup("AddComponentSearch")) {
			return;
		}

		static char searchBuffer[128] = "";

		ImGui::InputText("Search", searchBuffer, sizeof(searchBuffer));

		ImGui::Separator();

		auto available = ComponentRegistry::GetAvailableComponents(m_SelectedEntity);

		for (auto& desc : available) {
			if (IsInternalComponent(desc.id)) {
				continue;
			}

			if (strlen(searchBuffer) == 0 ||
				desc.displayName.find(searchBuffer) != std::string::npos) {

				if (ImGui::Selectable(desc.displayName.c_str())) {
					if (desc.id == entt::type_hash<NativeScriptComponent>::value() && m_Scene) {
						Entity scriptEntity = m_Scene->CreateAttachmentEntity(m_SelectedEntity);
						if (scriptEntity) {
							scriptEntity.AddComponent<NativeScriptComponent>();
						}
					} else {
						auto* descPtr = ComponentRegistry::GetComponentDescriptor(desc.id);
						if (descPtr && descPtr->addComponent) {
							descPtr->addComponent(m_SelectedEntity);
						}
					}
					memset(searchBuffer, 0, sizeof(searchBuffer));
					ImGui::CloseCurrentPopup();//关闭弹窗
					break;
				}
			}
		}

		ImGui::EndPopup();//结构对而已
	}

	void PropertiesPanel::DrawAttachedNativeScripts() {

		if (!m_Scene || !m_SelectedEntity) {
			return;
		}

		ImGui::Spacing();
		if (!ImGui::TreeNodeEx("Attached Native Scripts", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth)) {
			return;
		}

		std::vector<Entity> attachmentEntities = m_Scene->GetAttachmentEntities(m_SelectedEntity);
		for (Entity& attachmentEntity : attachmentEntities) {
			if (!attachmentEntity.HasComponents<NativeScriptComponent>()) {
				continue;
			}

			DrawNativeScriptComponentHeader(attachmentEntity, true, "NativeScriptComponent");
		}

		if (ImGui::Button("Add Attached Native Script", ImVec2(-1.0f, 0.0f))) {
			Entity scriptEntity = m_Scene->CreateAttachmentEntity(m_SelectedEntity);
			if (scriptEntity) {
				scriptEntity.AddComponent<NativeScriptComponent>();
			}
		}

		ImGui::TreePop();

	}

	void PropertiesPanel::DrawInternalComponents() {

		if (!m_SelectedEntity) {
			return;
		}

		bool hasInternalComponent = m_SelectedEntity.HasComponents<IDComponent>()
			|| m_SelectedEntity.HasComponents<RelationshipComponent>()
			|| m_SelectedEntity.HasComponents<AttachmentComponent>()
			|| m_SelectedEntity.HasComponents<HiddenEntityComponent>();

		if (!hasInternalComponent) {
			return;
		}

		ImGui::Spacing();
		if (!ImGui::TreeNodeEx("Internal", ImGuiTreeNodeFlags_SpanFullWidth)) {
			return;
		}

		if (m_SelectedEntity.HasComponents<IDComponent>()) {
			uint64_t id = m_SelectedEntity.GetComponents<IDComponent>().ID;
			ImGui::Text("UUID: %llu", static_cast<unsigned long long>(id));
		}

		if (m_SelectedEntity.HasComponents<RelationshipComponent>()) {
			auto& relationship = m_SelectedEntity.GetComponents<RelationshipComponent>();
			ImGui::SeparatorText("Relationship");
			ImGui::Text("ParentID: %llu", static_cast<unsigned long long>(relationship.ParentID));
			ImGui::Text("Children: %zu", relationship.ChildrenIDs.size());
			for (uint64_t childID : relationship.ChildrenIDs) {
				ImGui::BulletText("%llu", static_cast<unsigned long long>(childID));
			}
		}

		if (m_SelectedEntity.HasComponents<AttachmentComponent>()) {
			auto& attachment = m_SelectedEntity.GetComponents<AttachmentComponent>();
			ImGui::SeparatorText("Attachment");
			ImGui::Text("OwnerID: %llu", static_cast<unsigned long long>(attachment.OwnerID));
		}

		if (m_SelectedEntity.HasComponents<HiddenEntityComponent>()) {
			ImGui::SeparatorText("Hidden");
			ImGui::Text("Hidden: %s", m_SelectedEntity.GetComponents<HiddenEntityComponent>().Hidden ? "true" : "false");
		}

		ImGui::TreePop();

	}

	bool PropertiesPanel::IsInternalComponent(entt::id_type componentID) const {

		return componentID == entt::type_hash<IDComponent>::value()
			|| componentID == entt::type_hash<RelationshipComponent>::value()
			|| componentID == entt::type_hash<AttachmentComponent>::value()
			|| componentID == entt::type_hash<HiddenEntityComponent>::value();

	}

}
