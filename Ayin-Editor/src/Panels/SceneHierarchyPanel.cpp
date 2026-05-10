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

		ImGui::End();

	};


	void SceneHierarchyPanel::DrawEntityNode(Entity& node) {
		
		TagComponent tag = node.GetComponents<TagComponent>();

		bool isOpen = (node == m_SelectedEntity || node == m_HoveredEntity);
		ImGuiTreeNodeFlags nodeFlags = m_LeafNodeFlags | ((isOpen) ? ImGuiTreeNodeFlags_Selected : 0);

		ImGui::TreeNodeEx(tag.Name.data(), nodeFlags);

		if (ImGui::IsItemHovered()) {
			m_HoveredEntity = node;
		}

		if (ImGui::IsItemClicked()) {
			m_SelectedEntity = node;
		}

	};


}
