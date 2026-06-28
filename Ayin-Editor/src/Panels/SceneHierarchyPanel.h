#pragma once

#include <Ayin.h>

namespace Ayin {

	class SceneHierarchyPanel {

	public:

		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		~SceneHierarchyPanel() = default;

		void SetContext(const Ref<Scene>& scene);

		void OnImGuiRender();

		inline const Entity& GetSelectedEntity() const { return m_SelectedEntity; }

	private:

		void DrawEntityNode(Entity& node);	// 绘制可能执行删除，所以参数就不带const了

	private:

		Ref<Scene> m_Scene;
		Entity m_SelectedEntity, m_HoveredEntity;

											 //叶节点（无前缀图标）	   //选中时整行高亮					  //不开新树
		ImGuiTreeNodeFlags m_LeafNodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_DrawLinesFull;
											   //点开箭头展开						//层级线绘制						    //选中时整行高亮
		ImGuiTreeNodeFlags m_ParentNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DrawLinesFull | ImGuiTreeNodeFlags_SpanFullWidth;

		char m_SceneNameBuffer[256] = {};
		bool m_EditingSceneName = false;

	};

}
