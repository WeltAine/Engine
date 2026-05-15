#include <AyinPch.h>

#include "Ayin/Scene/ComponentRegistry.h"

#include <imgui.h>

namespace Ayin {

	std::vector<ComponentDescriptor>& ComponentRegistry::GetAllComponentDescriptors() {
		
		static std::vector<ComponentDescriptor> componentDescriptors = []() -> std::vector<ComponentDescriptor> 
			{
				std::vector<ComponentDescriptor> vector; 
				vector.reserve(50);
				return vector;
			}();

		return componentDescriptors;

	}


	void ComponentRegistry::DrawEntityComponents(Entity entity) {

		for (auto& componentDescriptor : GetAllComponentDescriptors()) {
			if (entity.HasComponent(componentDescriptor.id)) {
				// 可折叠标题栏（前面带一个三角箭头），返回值为折叠状态
				if (ImGui::CollapsingHeader(componentDescriptor.displayName.c_str())) {
					componentDescriptor.onGUI(entity);
				}
			}
		}

	}

	std::vector<ComponentDescriptor> ComponentRegistry::GetAvailableComponents(Entity entity) {

		std::vector<ComponentDescriptor> available;

		for (auto& componentDescriptor : GetAllComponentDescriptors()) {
			if (!entity.HasComponent(componentDescriptor.id)) {
				available.push_back(componentDescriptor);
			}
		}

		return available;
	}

	ComponentDescriptor* ComponentRegistry::GetComponentDescriptor(entt::id_type id) {

		auto& allComponentDescriptors = GetAllComponentDescriptors();
		for (auto& desc : allComponentDescriptors) {
			if (desc.id == id) {
				return &(desc);
			}
		}

		return nullptr;

	}

	ComponentDescriptor* ComponentRegistry::GetComponentDescriptorByName(const std::string& name) {

		auto& allComponentDescriptors = GetAllComponentDescriptors();
		for (auto& desc : allComponentDescriptors) {
			if (desc.displayName == name) {
				return &(desc);
			}
		}

		return nullptr;

	}


}
