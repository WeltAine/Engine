#include <AyinPch.h>

#include "OrbitGame.h"

#include "Ayin/Scene/SceneSerializer.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>

namespace {

	constexpr float s_TwoPi = 6.28318530718f;
	constexpr float s_PlayerCollisionRadius = 0.36f;
	constexpr float s_OrbiterCollisionRadius = 0.25f;
	constexpr float s_EnemyCollisionRadius = 0.34f;

	std::mt19937& GetRandomEngine() {

		static std::mt19937 s_RandomEngine{ std::random_device{}() };
		return s_RandomEngine;

	}

	float DistanceSquared(const glm::vec3& left, const glm::vec3& right) {

		glm::vec3 delta = left - right;
		return glm::dot(delta, delta);

	}

	bool IsCircleOverlap(const glm::vec3& left, float leftRadius, const glm::vec3& right, float rightRadius) {

		float radiusSum = leftRadius + rightRadius;
		return DistanceSquared(left, right) <= radiusSum * radiusSum;

	}

	glm::vec3 ClampToArena(const glm::vec3& position, float arenaHalfSize) {

		return {
			std::clamp(position.x, -arenaHalfSize, arenaHalfSize),
			std::clamp(position.y, -arenaHalfSize, arenaHalfSize),
			position.z
		};

	}

}

namespace OrbitGame {

	void OrbitEnemyComponent::OnGui(Ayin::Entity& entity) {

		auto& enemy = entity.GetComponents<OrbitEnemyComponent>();
		ImGui::DragFloat("Health", &enemy.Health, 0.01f, 0.0f, enemy.MaxHealth);
		ImGui::DragFloat("Max Health", &enemy.MaxHealth, 0.01f, 0.1f, 100.0f);

	}

	void OrbitCameraScript::OnUpdate(Ayin::Timestep deltaTime) {

		AYIN_PROFILE_FUNCTION();

		(void)deltaTime;
		auto&& [transform, camera] = GetComponents<Ayin::TransformComponent, Ayin::CameraComponent>();
		camera.Camera.SetViewMatrix(transform.Position, transform.Rotation);

	}

	void OrbitGameControllerScript::OnCreate() {

		ResetRuntimeState();

	}

	void OrbitGameControllerScript::OnUpdate(Ayin::Timestep deltaTime) {

		AYIN_PROFILE_FUNCTION();

		float seconds = std::min(deltaTime.GetSeconds(), 0.05f);
		EnsureCoreEntities();
		EnsureEnemyCount();

		UpdatePlayer(seconds);
		UpdateOrbiter(seconds);
		UpdateEnemies(seconds);
		ResolveCollisions(seconds);
		EnsureEnemyCount();

	}

	void OrbitGameControllerScript::OnGui() {

		ImGui::SeparatorText("Orbit Game Controller");
		ImGui::DragFloat("Player Speed", &m_PlayerSpeed, 0.05f, 0.1f, 20.0f);
		ImGui::DragFloat("Enemy Speed", &m_EnemySpeed, 0.05f, 0.1f, 20.0f);
		ImGui::DragFloat("Enemy Max Health", &m_EnemyMaxHealth, 0.05f, 0.1f, 20.0f);
		ImGui::DragFloat("Enemy Damage", &m_EnemyDamagePerSecond, 0.01f, 0.0f, 10.0f);
		ImGui::DragInt("Enemy Count", &m_TargetEnemyCount, 1, 0, 30);
		ImGui::DragFloat("Arena Half Size", &m_ArenaHalfSize, 0.1f, 2.0f, 30.0f);

		ImGui::SeparatorText("Orbiter");
		ImGui::DragFloat("Radius", &m_OrbiterRadius, 0.01f, m_OrbiterMinRadius, m_OrbiterMaxRadius);
		ImGui::DragFloat("Min Radius", &m_OrbiterMinRadius, 0.01f, 0.1f, m_OrbiterMaxRadius);
		ImGui::DragFloat("Max Radius", &m_OrbiterMaxRadius, 0.01f, m_OrbiterMinRadius, 10.0f);
		ImGui::DragFloat("Angular Velocity", &m_OrbiterAngularVelocity, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat("Decay Speed", &m_OrbiterRadiusDecaySpeed, 0.01f, 0.0f, 10.0f);
		ImGui::DragFloat("Expand Speed", &m_OrbiterRadiusExpandSpeed, 0.01f, 0.0f, 10.0f);

	}

	Ayin::Entity OrbitGameControllerScript::FindNamedEntity(const std::string& name) {

		Ayin::Scene* scene = GetScene();
		if (scene == nullptr) {
			return {};
		}

		auto entities = scene->GetEntitiesByComponents<Ayin::TagComponent>();
		for (Ayin::Entity entity : entities) {
			if (entity.GetComponents<Ayin::TagComponent>().Name == name) {
				return entity;
			}
		}

		return {};

	}

	std::vector<Ayin::Entity> OrbitGameControllerScript::GetEnemies() {

		Ayin::Scene* scene = GetScene();
		if (scene == nullptr) {
			return {};
		}

		return scene->GetEntitiesByComponents<OrbitEnemyComponent, Ayin::TransformComponent, Ayin::SpriteRendererComponent>();

	}

	glm::vec3 OrbitGameControllerScript::GenerateEnemySpawnPosition(const glm::vec3& playerPosition) {

		std::uniform_real_distribution<float> angleDistribution{ 0.0f, s_TwoPi };
		std::uniform_real_distribution<float> radiusDistribution{ 2.4f, std::max(2.5f, m_ArenaHalfSize) };

		for (int attempt = 0; attempt < 12; attempt++) {
			float angle = angleDistribution(GetRandomEngine());
			float radius = radiusDistribution(GetRandomEngine());
			glm::vec3 candidate{
				playerPosition.x + std::cos(angle) * radius,
				playerPosition.y + std::sin(angle) * radius,
				0.0f
			};

			candidate = ClampToArena(candidate, m_ArenaHalfSize);
			if (DistanceSquared(candidate, playerPosition) > 4.0f) {
				return candidate;
			}
		}

		return { m_ArenaHalfSize, 0.0f, 0.0f };

	}

	void OrbitGameControllerScript::EnsureCoreEntities() {

		Ayin::Scene* scene = GetScene();
		if (scene == nullptr) {
			return;
		}

		Ayin::Entity player = FindNamedEntity(PlayerName);
		if (!player) {
			player = scene->CreateEntity(PlayerName);
			player.GetComponents<Ayin::TransformComponent>().Scale = { 0.72f, 0.72f, 1.0f };
			player.AddComponent<Ayin::SpriteRendererComponent>().Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		Ayin::Entity orbiter = FindNamedEntity(OrbiterName);
		if (!orbiter) {
			orbiter = scene->CreateEntity(OrbiterName);
			orbiter.GetComponents<Ayin::TransformComponent>().Position = { m_OrbiterRadius, 0.0f, 0.1f };
			orbiter.GetComponents<Ayin::TransformComponent>().Scale = { 0.46f, 0.46f, 1.0f };
			orbiter.AddComponent<Ayin::SpriteRendererComponent>().Color = { 0.08f, 0.48f, 1.0f, 1.0f };
		}

		if (scene->GetParent(orbiter)) {
			// 旧版本把蓝块作为玩家子实体；现在它是独立卫星，运行时自动解父子关系。
			scene->Unparent(orbiter, true);
		}

	}

	void OrbitGameControllerScript::EnsureEnemyCount() {

		Ayin::Scene* scene = GetScene();
		if (scene == nullptr) {
			return;
		}

		Ayin::Entity player = FindNamedEntity(PlayerName);
		glm::vec3 playerPosition = player ? player.GetComponents<Ayin::TransformComponent>().Position : glm::vec3{ 0.0f };

		std::vector<Ayin::Entity> enemies = GetEnemies();
		while (static_cast<int>(enemies.size()) > m_TargetEnemyCount) {
			scene->DestroyEntity(enemies.back());
			enemies.pop_back();
		}

		while (static_cast<int>(enemies.size()) < m_TargetEnemyCount) {
			glm::vec3 spawnPosition = GenerateEnemySpawnPosition(playerPosition);
			Ayin::Entity enemy = CreateOrbitEnemy(*scene, EnemyPrefix + std::to_string(enemies.size()), spawnPosition, m_EnemyMaxHealth);
			enemies.push_back(enemy);
		}

	}

	void OrbitGameControllerScript::UpdatePlayer(float deltaTime) {

		Ayin::Entity player = FindNamedEntity(PlayerName);
		if (!player) {
			return;
		}

		auto& transform = player.GetComponents<Ayin::TransformComponent>();
		glm::vec3 moveDirection{ 0.0f };

		if (Ayin::Input::IsKeyPressed(AYIN_KEY_A)) { moveDirection.x -= 1.0f; }
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_D)) { moveDirection.x += 1.0f; }
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_W)) { moveDirection.y += 1.0f; }
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_S)) { moveDirection.y -= 1.0f; }

		if (glm::dot(moveDirection, moveDirection) > 0.0f) {
			moveDirection = glm::normalize(moveDirection);
			transform.Position += moveDirection * m_PlayerSpeed * deltaTime;
			transform.Position = ClampToArena(transform.Position, m_ArenaHalfSize);
		}

	}

	void OrbitGameControllerScript::UpdateOrbiter(float deltaTime) {

		Ayin::Entity player = FindNamedEntity(PlayerName);
		Ayin::Entity orbiter = FindNamedEntity(OrbiterName);
		if (!player || !orbiter) {
			return;
		}

		auto& transform = orbiter.GetComponents<Ayin::TransformComponent>();
		glm::vec3 playerPosition = player.GetComponents<Ayin::TransformComponent>().Position;
		bool pulling = Ayin::Input::IsKeyPressed(AYIN_KEY_SPACE);
		m_OrbiterRadialVelocity = (pulling ? m_OrbiterRadiusExpandSpeed : 0.0f) - m_OrbiterRadiusDecaySpeed;
		m_OrbiterRadius = std::clamp(m_OrbiterRadius + m_OrbiterRadialVelocity * deltaTime, m_OrbiterMinRadius, m_OrbiterMaxRadius);

		float radiusForSpeed = std::max(m_OrbiterRadius, 0.001f);
		float speedScale = std::sqrt(std::max(m_OrbiterMinRadius, m_OrbiterMaxRadius) / radiusForSpeed);
		float angularVelocity = m_OrbiterAngularVelocity * speedScale;
		m_OrbiterAngle += angularVelocity * deltaTime;
		if (m_OrbiterAngle > s_TwoPi || m_OrbiterAngle < -s_TwoPi) {
			m_OrbiterAngle = std::fmod(m_OrbiterAngle, s_TwoPi);
		}

		m_OrbiterSelfRotation = std::fmod(m_OrbiterSelfRotation + (160.0f + std::abs(angularVelocity) * 55.0f) * deltaTime, 360.0f);
		transform.Position = playerPosition + glm::vec3{ std::cos(m_OrbiterAngle) * m_OrbiterRadius, std::sin(m_OrbiterAngle) * m_OrbiterRadius, 0.1f };
		transform.Rotation = { 0.0f, 0.0f, m_OrbiterSelfRotation };

	}

	void OrbitGameControllerScript::UpdateEnemies(float deltaTime) {

		Ayin::Entity player = FindNamedEntity(PlayerName);
		if (!player) {
			return;
		}

		glm::vec3 playerPosition = player.GetComponents<Ayin::TransformComponent>().Position;
		for (Ayin::Entity enemy : GetEnemies()) {
			auto& transform = enemy.GetComponents<Ayin::TransformComponent>();
			glm::vec3 chaseDirection = playerPosition - transform.Position;
			chaseDirection.z = 0.0f;

			if (glm::dot(chaseDirection, chaseDirection) > 0.0001f) {
				transform.Position += glm::normalize(chaseDirection) * m_EnemySpeed * deltaTime;
				transform.Position = ClampToArena(transform.Position, m_ArenaHalfSize);
			}

			auto& enemyData = enemy.GetComponents<OrbitEnemyComponent>();
			auto& sprite = enemy.GetComponents<Ayin::SpriteRendererComponent>();
			sprite.Color.a = std::clamp(enemyData.Health / std::max(0.1f, enemyData.MaxHealth), 0.12f, 1.0f);
		}

	}

	void OrbitGameControllerScript::ResolveCollisions(float deltaTime) {

		Ayin::Scene* scene = GetScene();
		Ayin::Entity player = FindNamedEntity(PlayerName);
		Ayin::Entity orbiter = FindNamedEntity(OrbiterName);
		if (scene == nullptr || !player || !orbiter) {
			return;
		}

		glm::vec3 playerPosition = player.GetComponents<Ayin::TransformComponent>().Position;
		glm::vec3 orbiterWorldPosition = orbiter.GetComponents<Ayin::TransformComponent>().Position;

		if (IsCircleOverlap(playerPosition, s_PlayerCollisionRadius, orbiterWorldPosition, s_OrbiterCollisionRadius)) {
			KillPlayer();
			return;
		}

		std::vector<Ayin::Entity> defeatedEnemies;
		for (Ayin::Entity enemy : GetEnemies()) {
			glm::vec3 enemyPosition = enemy.GetComponents<Ayin::TransformComponent>().Position;
			if (!IsCircleOverlap(orbiterWorldPosition, s_OrbiterCollisionRadius, enemyPosition, s_EnemyCollisionRadius)) {
				continue;
			}

			auto& enemyData = enemy.GetComponents<OrbitEnemyComponent>();
			enemyData.Health -= m_EnemyDamagePerSecond * deltaTime;
			if (enemyData.Health <= 0.0f) {
				defeatedEnemies.push_back(enemy);
			}
		}

		for (Ayin::Entity enemy : defeatedEnemies) {
			scene->DestroyEntity(enemy);
		}

	}

	void OrbitGameControllerScript::RespawnEnemies() {

		Ayin::Scene* scene = GetScene();
		if (scene == nullptr) {
			return;
		}

		std::vector<Ayin::Entity> enemies = GetEnemies();
		for (Ayin::Entity enemy : enemies) {
			scene->DestroyEntity(enemy);
		}

		EnsureEnemyCount();

	}

	void OrbitGameControllerScript::KillPlayer() {

		Ayin::Entity player = FindNamedEntity(PlayerName);
		Ayin::Entity orbiter = FindNamedEntity(OrbiterName);
		if (!player || !orbiter) {
			return;
		}

		m_PlayerDead = true;
		m_OrbiterRadius = 2.1f;
		m_OrbiterAngle = 0.0f;
		m_OrbiterRadialVelocity = 0.0f;

		player.GetComponents<Ayin::TransformComponent>().Position = { 0.0f, 0.0f, 0.0f };
		orbiter.GetComponents<Ayin::TransformComponent>().Position = { m_OrbiterRadius, 0.0f, 0.1f };

		RespawnEnemies();
		m_PlayerDead = false;

	}

	void OrbitGameControllerScript::ResetRuntimeState() {

		EnsureCoreEntities();

		m_PlayerDead = false;
		m_OrbiterRadius = std::clamp(m_OrbiterRadius, m_OrbiterMinRadius, m_OrbiterMaxRadius);

		Ayin::Entity player = FindNamedEntity(PlayerName);
		Ayin::Entity orbiter = FindNamedEntity(OrbiterName);
		if (player && orbiter) {
			glm::vec3 playerPosition = player.GetComponents<Ayin::TransformComponent>().Position;
			orbiter.GetComponents<Ayin::TransformComponent>().Position = playerPosition + glm::vec3{ std::cos(m_OrbiterAngle) * m_OrbiterRadius, std::sin(m_OrbiterAngle) * m_OrbiterRadius, 0.1f };
		}
		EnsureEnemyCount();

	}

	Ayin::Ref<Ayin::Scene> CreateOrbitCombatScene() {

		auto scene = Ayin::CreateRef<Ayin::Scene>();
		PopulateOrbitCombatScene(scene);
		return scene;

	}

	Ayin::Ref<Ayin::Scene> LoadOrbitCombatScene() {

		if (std::filesystem::exists(OrbitCombatScenePath)) {
			std::ifstream input{ OrbitCombatScenePath };
			if (input.is_open()) {
				std::stringstream buffer;
				buffer << input.rdbuf();

				auto scene = Ayin::CreateRef<Ayin::Scene>();
				Ayin::SceneSerializer sceneSerializer{ scene };
				if (sceneSerializer.DeserializeFromString(buffer.str())) {
					return scene;
				}
			}
		}

		return CreateOrbitCombatScene();

	}

	void PopulateOrbitCombatScene(const Ayin::Ref<Ayin::Scene>& scene) {

		if (!scene) {
			return;
		}

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

		Ayin::Entity player = scene->CreateEntity(PlayerName);
		player.GetComponents<Ayin::TransformComponent>().Scale = { 0.72f, 0.72f, 1.0f };
		player.AddComponent<Ayin::SpriteRendererComponent>().Color = { 1.0f, 1.0f, 1.0f, 1.0f };

		Ayin::Entity orbiter = scene->CreateEntity(OrbiterName);
		orbiter.GetComponents<Ayin::TransformComponent>().Position = { 2.1f, 0.0f, 0.1f };
		orbiter.GetComponents<Ayin::TransformComponent>().Scale = { 0.46f, 0.46f, 1.0f };
		orbiter.AddComponent<Ayin::SpriteRendererComponent>().Color = { 0.08f, 0.48f, 1.0f, 1.0f };

		Ayin::Entity controller = scene->CreateEntity(ControllerName);
		controller.AddScript<OrbitGameControllerScript>();

		CreateOrbitEnemy(*scene, "Enemy0", { -3.8f, 2.4f, 0.0f }, 1.0f);
		CreateOrbitEnemy(*scene, "Enemy1", { 3.2f, 2.8f, 0.0f }, 1.0f);
		CreateOrbitEnemy(*scene, "Enemy2", { -3.1f, -2.9f, 0.0f }, 1.0f);
		CreateOrbitEnemy(*scene, "Enemy3", { 3.9f, -1.8f, 0.0f }, 1.0f);
		CreateOrbitEnemy(*scene, "Enemy4", { 0.0f, 4.6f, 0.0f }, 1.0f);

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
