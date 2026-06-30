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

				if(!entity.GetParent())	// 根实体
					DrawEntityNode(entity);
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
			if (open) {
				drawEntities();
				ImGui::TreePop();
			}

		} else {
			// 正常态：场景名直接作 TreeNode label，保留 SpanFullWidth 实现整行高亮
			bool open = ImGui::TreeNodeEx(m_Scene->GetName().c_str(), sceneFlags);

			// 检查是否对场景进行编辑
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {// 悬浮且双击切换状态
				m_EditingSceneName = true;
				strncpy(m_SceneNameBuffer, m_Scene->GetName().c_str(), sizeof(m_SceneNameBuffer) - 1);
			}

			// 如果场景节点展开
			if (open) {
				drawEntities();
				ImGui::TreePop();
			}

		}

		// 相比于ImGui::BeginPopup，这个则是通过鼠标活动来打开弹窗
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

		bool isHightLight = (node == m_SelectedEntity || node == m_HoveredEntity);
		
		std::vector<Entity> childEntities = node.GetChilds();
		ImGuiTreeNodeFlags nodeFlags = (childEntities.empty() ? m_LeafNodeFlags : m_ParentNodeFlags) | ((isHightLight) ? ImGuiTreeNodeFlags_Selected : 0);

		//! 支持重名 Entity
		std::string label = fmt::format("{}##{}", tag.Name, node.GetComponents<IDComponent>().ID);
		bool open = ImGui::TreeNodeEx(label.c_str(), nodeFlags);	//  open 表示节点处于打开/可提交子内容状态；但带 NoTreePushOnOpen 的叶子节点不会 push tree stack

		//! 关于 Item 的相关接口，我们提到前面了，因为这些接口中的 Item 是什么，取决于此前 ImGui 绘制指令中提交的最后一个 Item
		//! 如果放到递归之后，那么可能出现 此时检测的 Item 并非这个函数栈帧中绘制的那个节点
		if (ImGui::IsItemHovered()) {
			m_HoveredEntity = node;
		}

		if (ImGui::IsItemClicked(0)) {
			m_SelectedEntity = node;
		}

		if (ImGui::BeginPopupContextItem()) {

			if (ImGui::MenuItem("Delete Entity")) {

				if (node == m_SelectedEntity || m_Scene->IsDescendant(m_SelectedEntity, node)) {// 选中的节点也被删了
					m_SelectedEntity = {};
				}

				m_Scene->DestroyEntity(node);

			}

			ImGui::EndPopup();

		}

		//! 真展开了一定 open，但是 open 不代表一定展开（叶子节点不压入缩进）
		if (open && !(nodeFlags & ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
			for(auto& child : childEntities)
				DrawEntityNode(child);

			ImGui::TreePop();
		}

	};


}
