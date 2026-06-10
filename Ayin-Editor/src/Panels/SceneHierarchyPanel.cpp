#include <AyinPch.h>

#include "Panels/SceneHierarchyPanel.h"

#include <algorithm>


namespace Ayin {

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
		:m_Scene{scene} 
	{};


	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene) { m_Scene = scene; m_SelectedEntity = {}; m_HoveredEntity = {}; };


	void SceneHierarchyPanel::OnImGuiRender() {
	
		std::vector<Entity> entitys = m_Scene->GetEntitiesByComponents<TagComponent>(entt::exclude<HiddenEntityComponent>);

		ImGui::Begin("HierarchyPanel");

		ImGuiTreeNodeFlags sceneFlags = m_ParentNodeFlags | ImGuiTreeNodeFlags_DefaultOpen;

		auto drawEntities = [&]() {
			for (Entity& entity : entitys) {
				if (!m_Scene->GetParent(entity)) {
					DrawEntityNode(entity);
				}
			}
		};

		if (m_EditingSceneName) {
			// 编辑态：去掉 SpanFullWidth，否则 TreeNode 的交互区域会铺满整行，
			// 遮挡后面 SameLine() 上的 InputText，导致其无法接收鼠标事件
			sceneFlags &= ~ImGuiTreeNodeFlags_SpanFullWidth;//去掉整行高亮

			bool open = ImGui::TreeNodeEx("##SceneRoot", sceneFlags);//无场景名称

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

			if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0))
				m_EditingSceneName = false;

			//绘制场景中节点
			DrawSceneRootDropTarget();

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

			DrawSceneRootDropTarget();

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

		if (!node.HasComponents<TagComponent>() || node.HasComponents<HiddenEntityComponent>()) {
			return;
		}

		TagComponent tag = node.GetComponents<TagComponent>();
		std::vector<Entity> children = m_Scene->GetChildren(node);
		children.erase(
			std::remove_if(children.begin(), children.end(), [](Entity& child) {
				return !child.HasComponents<TagComponent>() || child.HasComponents<HiddenEntityComponent>();
			}),
			children.end()
		);

		bool isHightLight = (node == m_SelectedEntity || node == m_HoveredEntity);
		bool hasChildren = !children.empty();
		bool deleted = false;
		ImGuiTreeNodeFlags nodeFlags = (hasChildren ? m_ParentNodeFlags : m_LeafNodeFlags) | ((isHightLight) ? ImGuiTreeNodeFlags_Selected : 0);

		ImGui::PushID(static_cast<uint32_t>(node));

		bool open = ImGui::TreeNodeEx(tag.Name.data(), nodeFlags);

		if (ImGui::IsItemHovered()) {
			m_HoveredEntity = node;
		}

		if (ImGui::IsItemClicked(0)) {
			m_SelectedEntity = node;
		}

		DrawEntityDragDrop(node);

		if (ImGui::BeginPopupContextItem()) {

			if (ImGui::MenuItem("Create Child Entity")) {

				Entity child = m_Scene->CreateEntity();
				m_Scene->SetParent(child, node, false);
				m_SelectedEntity = child;

			}

			ImGui::Separator();

			if (ImGui::MenuItem("Delete Entity")) {

				m_SelectedEntity = {};
				m_HoveredEntity = {};

				m_Scene->DestroyEntity(node);
				deleted = true;

			}

			ImGui::EndPopup();

		}

		if (open && hasChildren) {
			if (!deleted) {
				for (Entity& child : children) {
					DrawEntityNode(child);
				}
			}
			ImGui::TreePop();
		}

		ImGui::PopID();

	};

	void SceneHierarchyPanel::DrawSceneRootDropTarget() {

		if (!ImGui::BeginDragDropTarget()) {
			return;
		}

		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AYIN_ENTITY")) {
			AYIN_CORE_ASSERT(payload->DataSize == sizeof(entt::entity), "Invalid entity drag payload");
			entt::entity entityHandle = *static_cast<entt::entity*>(payload->Data);
			Entity draggedEntity{ entityHandle, m_Scene.get() };

			if (draggedEntity) {
				m_Scene->Unparent(draggedEntity);
			}
		}

		ImGui::EndDragDropTarget();

	}

	void SceneHierarchyPanel::DrawEntityDragDrop(Entity& node) {

		if (ImGui::BeginDragDropSource()) {
			entt::entity entityHandle = node;
			ImGui::SetDragDropPayload("AYIN_ENTITY", &entityHandle, sizeof(entityHandle));

			if (node.HasComponents<TagComponent>()) {
				ImGui::Text("%s", node.GetComponents<TagComponent>().Name.c_str());
			} else {
				ImGui::Text("Entity");
			}

			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AYIN_ENTITY")) {
				AYIN_CORE_ASSERT(payload->DataSize == sizeof(entt::entity), "Invalid entity drag payload");
				entt::entity entityHandle = *static_cast<entt::entity*>(payload->Data);
				Entity draggedEntity{ entityHandle, m_Scene.get() };

				if (draggedEntity && draggedEntity != node) {
					m_Scene->SetParent(draggedEntity, node);
				}
			}

			ImGui::EndDragDropTarget();
		}

	}


}
