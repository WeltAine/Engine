#include <AyinPch.h>

#include "DemoScripts.h"

namespace DemoScripts {

	void TutorialSceneControllerScript::OnUpdate(Ayin::Timestep deltaTime) {

		AYIN_PROFILE_FUNCTION();
		float seconds = deltaTime.GetSeconds();
		m_ElapsedSeconds += seconds;
		auto& transform = GetComponents<Ayin::TransformComponent>();
		transform.Rotation.z += m_RotationSpeed * seconds;

	}

	void TutorialSceneControllerScript::OnGui() {

		ImGui::SeparatorText("Tutorial Scene Controller");
		ImGui::DragFloat("Rotation Speed", &m_RotationSpeed, 0.1f, -360.0f, 360.0f);
		ImGui::Text("Elapsed: %.2f", m_ElapsedSeconds);

	}

}
