#include <AyinPch.h>

#include "Panels/PropertiesPanel.h"

namespace Ayin {

	void PropertiesPanel::OnImGuiRender() {

		ImGui::Begin("Properties");

		if (m_SelectedEntity) {

			auto&& descriptors = ComponentRegistry::GetAllComponentDescriptors();

			ImGui::PushID(m_SelectedEntity.GetComponents<TagComponent>().Name.c_str());

			for (size_t i = 0; i < descriptors.size(); ++i) {
				auto& desc = descriptors[i];
				if (m_SelectedEntity.HasComponent(desc.id)) {
					DrawComponentHeader(desc);

					ImGui::Separator();
				}
			}

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
			if (strlen(searchBuffer) == 0 ||
				desc.displayName.find(searchBuffer) != std::string::npos) {

				if (ImGui::Selectable(desc.displayName.c_str())) {
					auto* descPtr = ComponentRegistry::GetComponentDescriptor(desc.id);
					if (descPtr && descPtr->addComponent) {
						descPtr->addComponent(m_SelectedEntity);
					}
					memset(searchBuffer, 0, sizeof(searchBuffer));
					ImGui::CloseCurrentPopup();//关闭弹窗
					break;
				}
			}
		}

		ImGui::EndPopup();//结构对而已
	}

}
