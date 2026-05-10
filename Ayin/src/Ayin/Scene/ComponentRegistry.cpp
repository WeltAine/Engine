#include <AyinPch.h>

#include "Ayin/Scene/ComponentRegistry.h"

#include <imgui.h>

namespace Ayin {

	std::vector<ComponentDescriptor>& ComponentRegistry::GetAllComponentDescriptors() {
		static std::vector<ComponentDescriptor> entries;
		return entries;
	}


	void ComponentRegistry::DrawEntityComponents(Entity entity) {

		for (auto& componentDescriptor : GetAllComponentDescriptors()) {
			if (entity.HasComponent(componentDescriptor.id)) {
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

}
