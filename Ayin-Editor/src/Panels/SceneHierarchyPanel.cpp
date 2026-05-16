#include <AyinPch.h>

#include "Panels/SceneHierarchyPanel.h"


namespace Ayin {

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
		:m_Scene{scene} 
	{};


	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene) { m_Scene = scene; m_SelectedEntity = {}; m_HoveredEntity = {}; };


	void SceneHierarchyPanel::OnImGuiRender() {
	
		std::vector<Entity> entitys = m_Scene->GetEntitiesByComponents<TagComponent>();

		ImGui::Begin("HierarchyPanel");

		ImGuiTreeNodeFlags sceneFlags = m_ParentNodeFlags | ImGuiTreeNodeFlags_DefaultOpen;

		auto drawEntities = [&]() {
			for (Entity& entity : entitys) {
				DrawEntityNode(entity);
			}
		};

		if (m_EditingSceneName) {
			// 编辑态：去掉 SpanFullWidth，否则 TreeNode 的交互区域会铺满整行，
			// 遮挡后面 SameLine() 上的 InputText，导致其无法接收鼠标事件
			sceneFlags &= ~ImGuiTreeNodeFlags_SpanFullWidth;//去掉整行高亮

			bool open = ImGui::TreeNodeEx("##SceneRoot", sceneFlags);

			//编辑框绘制
			ImGui::SameLine();
			ImGui::SetKeyboardFocusHere();//让下一个绘制的组件获取焦点，本帧组件变为活跃
			ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
			if (ImGui::InputText("##SceneNameEdit", m_SceneNameBuffer, sizeof(m_SceneNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
				m_Scene->SetName(m_SceneNameBuffer);
				m_EditingSceneName = false;
			}
			ImGui::PopStyleColor(3);

			if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0))//! m_JustStartedEditing是用于堵住时序漏洞的，因为第一次进入这里时
				m_EditingSceneName = false;

			//绘制场景中节点
			if (open) {
				drawEntities();
				ImGui::TreePop();
			}

		} else {
			// 正常态：场景名直接作 TreeNode label，保留 SpanFullWidth 实现整行高亮
			bool open = ImGui::TreeNodeEx(m_Scene->GetName().c_str(), sceneFlags);

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
				m_EditingSceneName = true;
				strncpy(m_SceneNameBuffer, m_Scene->GetName().c_str(), sizeof(m_SceneNameBuffer) - 1);
			}

			if (open) {
				drawEntities();
				ImGui::TreePop();
			}

		}

		if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{

			if (ImGui::MenuItem("Create Entity")) {

				m_Scene->CreateEntity();

			}

			ImGui::EndPopup();
		}

		ImGui::End();


		if (!ImGui::IsWindowHovered()) {
			m_HoveredEntity = {};
		}

	};


	void SceneHierarchyPanel::DrawEntityNode(Entity& node) {
		
		TagComponent tag = node.GetComponents<TagComponent>();

		bool isOpen = (node == m_SelectedEntity || node == m_HoveredEntity);
		ImGuiTreeNodeFlags nodeFlags = m_LeafNodeFlags | ((isOpen) ? ImGuiTreeNodeFlags_Selected : 0);

		ImGui::TreeNodeEx(tag.Name.data(), nodeFlags);

		if (ImGui::IsItemHovered()) {
			m_HoveredEntity = node;
		}

		if (ImGui::IsItemClicked(0)) {
			m_SelectedEntity = node;
		}

		if (ImGui::BeginPopupContextItem()) {

			if (ImGui::MenuItem("Delete Entity")) {

				if (node == m_SelectedEntity) {
					m_SelectedEntity = {};
				}

				m_Scene->DestroyEntity(node);

			}

			ImGui::EndPopup();

		}

	};


}
