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

			// 拖拽响应（编辑名称的时候也不可能拖拽吧）
			DrawSceneRootDropTarget();

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

			// 拖拽响应
			DrawSceneRootDropTarget();

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
		
		// 被拖拽则变色
		bool isDragged = IsEntityBeingDragged(node);
		if(isDragged)
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.45f);

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

		// 场景拖拽响应
		DrawEntityDragDrop(node);

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

		if (isDragged)
			ImGui::PopStyleVar();

	};

	void SceneHierarchyPanel::DrawEntityNodePreview(const Entity& node) {
		
		TagComponent tag = node.GetComponents<TagComponent>();

		std::vector<Entity> childEntities = node.GetChilds();
		ImGuiTreeNodeFlags nodeFlags = (childEntities.empty() ? m_LeafNodeFlags : m_ParentNodeFlags | ImGuiTreeNodeFlags_DefaultOpen);// 完全展开

		//! 支持重名 Entity
		std::string label = fmt::format("{}##{}", tag.Name, node.GetComponents<IDComponent>().ID);
		bool open = ImGui::TreeNodeEx(label.c_str(), nodeFlags);	//  open 表示节点处于打开/可提交子内容状态；但带 NoTreePushOnOpen 的叶子节点不会 push tree stack

		//! 真展开了一定 open，但是 open 不代表一定展开（叶子节点不压入缩进）
		if (open && !(nodeFlags & ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
			for (auto& child : childEntities)
				DrawEntityNodePreview(child);

			ImGui::TreePop();
		}

	};

	void SceneHierarchyPanel::DrawSceneRootDropTarget() {
	
		if (ImGui::BeginDragDropTarget()) {

			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SceneDraggedNodeHandle")) {
				//! 当前目标愿意接收，会发出“释放”动作（当然还需要我们松手）
				//! ImGui::GetDragDropPayload(); 也返回 const ImGuiPayload* ，但不会“释放”，只是用来观测
				//! ImGui 同一时间通常只有一个 active drag payload。也就是说，人手一次只能拖一个东西，

				Entity draggedEntity{ *(::entt::entity*)(payload->Data), m_Scene.get() };
				m_Scene->UnParent(draggedEntity, true);

			}

			ImGui::EndDragDropTarget();
		}

	
	};




	void SceneHierarchyPanel::DrawEntityDragDrop(Entity& node) {
	
		if (ImGui::BeginDragDropSource()) {//! 当前 UI 可拖拽
		
			::entt::entity handle = node;

			ImGui::SetDragDropPayload("SceneDraggedNodeHandle", &handle, sizeof(handle));//! 这里不只是记录地址，而是复制内存

			DrawEntityNodePreview(node);//这里估计会触发ImGui ID重复问题
		
			ImGui::EndDragDropSource();
		}

		
		if (ImGui::BeginDragDropTarget()) {//! 当前 UI 可被放置

			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SceneDraggedNodeHandle")) {
				//! 当前目标愿意接收，会发出“释放”动作（当然还需要我们松手）
				//! ImGui::GetDragDropPayload(); 也返回 const ImGuiPayload* ，但不会“释放”，只是用来观测
				//! ImGui 同一时间通常只有一个 active drag payload。也就是说，人手一次只能拖一个东西，
				
				Entity draggedEntity{*(::entt::entity*)(payload->Data), m_Scene.get()};
				m_Scene->SetParent(draggedEntity, node, true);
			
			}
		
			ImGui::EndDragDropTarget();
		}

	};

	bool SceneHierarchyPanel::IsEntityBeingDragged(const Entity& entity) const {

		const ImGuiPayload* payload = ImGui::GetDragDropPayload();

		if (!payload || !payload->IsDataType("SceneDraggedNodeHandle"))
			return false;

		if (payload->DataSize != sizeof(entt::entity))
			return false;

		entt::entity draggedHandle = *static_cast<const entt::entity*>(payload->Data);
		return draggedHandle == static_cast<entt::entity>(entity);

	}

}
