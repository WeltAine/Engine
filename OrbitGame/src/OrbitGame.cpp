#include <AyinPch.h>

#include "OrbitGame.h"

#include <algorithm>

namespace OrbitGame {

	void OrbitEnemyComponent::OnGui(Ayin::Entity& entity) {

		auto& enemy = entity.GetComponents<OrbitEnemyComponent>();
		ImGui::DragFloat("Health", &enemy.Health, 0.01f, 0.0f, enemy.MaxHealth);
		ImGui::DragFloat("Max Health", &enemy.MaxHealth, 0.01f, 0.1f, 100.0f);

	}

	void OrbitCameraScript::OnUpdate(Ayin::Timestep deltaTime) {

		AYIN_PROFILE_FUNCTION();

		auto&& [transform, camera] = GetComponents<Ayin::TransformComponent, Ayin::CameraComponent>();
		camera.Camera.SetViewMatrix(transform.Position, transform.Rotation);

	}

	void OrbitGameControllerScript::OnCreate() {

		ResetRuntimeState();

	}

	void OrbitGameControllerScript::OnUpdate(Ayin::Timestep deltaTime) {

		AYIN_PROFILE_FUNCTION();

		// 玩法逻辑会在下一阶段补齐。先保留脚本入口，确保工程骨架可编译、可注册、可序列化。
		(void)deltaTime;

	}

	void OrbitGameControllerScript::OnGui() {

		ImGui::SeparatorText("Orbit Game Controller");
		ImGui::DragFloat("Player Speed", &m_PlayerSpeed, 0.05f, 0.1f, 20.0f);
		ImGui::DragFloat("Enemy Speed", &m_EnemySpeed, 0.05f, 0.1f, 20.0f);
		ImGui::DragFloat("Enemy Damage", &m_EnemyDamagePerSecond, 0.01f, 0.0f, 10.0f);
		ImGui::DragInt("Enemy Count", &m_TargetEnemyCount, 1, 0, 30);
		ImGui::DragFloat("Orbiter Radius", &m_OrbiterRadius, 0.01f, m_OrbiterMinRadius, m_OrbiterMaxRadius);

	}

	void OrbitGameControllerScript::ResetRuntimeState() {

		Ayin::Scene* scene = GetScene();
		if (scene == nullptr) {
			return;
		}

		auto entities = scene->GetEntitiesByComponents<Ayin::TagComponent, Ayin::TransformComponent>();
		for (Ayin::Entity entity : entities) {
			if (entity.GetComponents<Ayin::TagComponent>().Name == PlayerName) {
				m_LastPlayerPosition = entity.GetComponents<Ayin::TransformComponent>().Position;
				return;
			}
		}

		m_LastPlayerPosition = { 0.0f, 0.0f, 0.0f };

	}

	void RegisterScripts() {

		Ayin::ScriptRegistry::Registry<OrbitCameraScript>("OrbitCameraScript");
		Ayin::ScriptRegistry::Registry<OrbitGameControllerScript>("OrbitGameControllerScript");

	}

	Ayin::Ref<Ayin::Scene> CreateOrbitCombatScene() {

		auto scene = Ayin::CreateRef<Ayin::Scene>();
		PopulateOrbitCombatScene(scene);
		return scene;

	}

	void PopulateOrbitCombatScene(const Ayin::Ref<Ayin::Scene>& scene) {

		if (!scene) {
			return;
		}

		RegisterScripts();
		scene->SetName("Orbit Combat Demo");

		Ayin::Entity mainCamera = scene->CreateEntity(MainCameraName);
		mainCamera.GetComponents<Ayin::TransformComponent>().Position = { 0.0f, 0.0f, 10.0f };
		mainCamera.AddComponent<Ayin::CameraComponent>(Ayin::CameraProp{
			.Type = Ayin::Camera::CameraType::Orthogonal,
			.Height = 12.0f,
			.AspectRatio = 16.0f / 9.0f,
			.NearPlaneDistance = 0.1f,
			.FarPlaneDistance = 100.0f
		});
		mainCamera.AddScript<OrbitCameraScript>();

		Ayin::Entity controller = scene->CreateEntity(ControllerName);
		controller.AddScript<OrbitGameControllerScript>();

	}

	Ayin::Entity CreateOrbitEnemy(Ayin::Scene& scene, const std::string& name, const glm::vec3& position, float maxHealth) {

		Ayin::Entity enemy = scene.CreateEntity(name);
		enemy.GetComponents<Ayin::TransformComponent>().Position = position;
		enemy.GetComponents<Ayin::TransformComponent>().Scale = { 0.62f, 0.62f, 1.0f };
		enemy.AddComponent<Ayin::SpriteRendererComponent>().Color = { 1.0f, 0.12f, 0.08f, 1.0f };
		enemy.AddComponent<OrbitEnemyComponent>(std::max(0.1f, maxHealth));

		return enemy;

	}

}
