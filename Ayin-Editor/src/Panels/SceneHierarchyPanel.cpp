#include <AyinPch.h>

#include "Panels/SceneHierarchyPanel.h"


namespace Ayin {

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
		:m_Scene{scene} 
	{};


	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene) { m_Scene = scene; };


	void SceneHierarchyPanel::OnImGuiRender() {
	
		std::vector<Entity> entitys = m_Scene->GetEntitiesByComponents<TagComponent>();

		ImGui::Begin("HierarchyPanel");

		if (ImGui::TreeNodeEx("TestScene", m_ParentNodeFlags | ImGuiTreeNodeFlags_DefaultOpen)) {

			for (Entity& entity : entitys) {

				DrawEntityNode(entity);

			}

			ImGui::TreePop();
		
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
