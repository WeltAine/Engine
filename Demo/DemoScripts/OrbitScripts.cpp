#include <AyinPch.h>

#include "DemoScripts.h"

#include <algorithm>
#include <cmath>

namespace {

	constexpr float s_TwoPi = 6.28318530718f;
	constexpr float s_PlayerCollisionRadius = 0.36f;
	constexpr float s_OrbiterCollisionRadius = 0.25f;
	constexpr float s_EnemyCollisionRadius = 0.34f;

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

namespace DemoScripts {

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
		UpdatePlayer(seconds);
		glm::vec3 playerPosition = m_PlayerPosition;
		UpdateOrbiter(seconds);
		UpdateEnemies(seconds, playerPosition);
		ResolveCollisions(playerPosition, seconds);
		DrawOrbitScene();

	}

	void OrbitGameControllerScript::OnGui() {

		ImGui::SeparatorText("Orbit Game Controller");
		ImGui::DragFloat("Player Speed", &m_PlayerSpeed, 0.05f, 0.1f, 20.0f);
		ImGui::DragFloat("Enemy Speed", &m_EnemySpeed, 0.05f, 0.1f, 20.0f);
		ImGui::DragFloat("Enemy Max Health", &m_EnemyMaxHealth, 0.05f, 0.1f, 20.0f);
		ImGui::DragFloat("Enemy Damage", &m_EnemyDamagePerSecond, 0.01f, 0.0f, 10.0f);
		ImGui::DragInt("Enemy Count", &m_TargetEnemyCount, 1, 0, OrbitMaxEnemyCount);
		ImGui::DragFloat("Arena Half Size", &m_ArenaHalfSize, 0.1f, 2.0f, 30.0f);
		ImGui::SeparatorText("Orbiter");
		ImGui::DragFloat("Radius", &m_OrbiterRadius, 0.01f, m_OrbiterMinRadius, m_OrbiterMaxRadius);
		ImGui::DragFloat("Min Radius", &m_OrbiterMinRadius, 0.01f, 0.1f, m_OrbiterMaxRadius);
		ImGui::DragFloat("Max Radius", &m_OrbiterMaxRadius, 0.01f, m_OrbiterMinRadius, 10.0f);
		ImGui::DragFloat("Angular Velocity", &m_OrbiterAngularVelocity, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat("Decay Speed", &m_OrbiterRadiusDecaySpeed, 0.01f, 0.0f, 10.0f);
		ImGui::DragFloat("Expand Speed", &m_OrbiterRadiusExpandSpeed, 0.01f, 0.0f, 10.0f);

	}

	void OrbitGameControllerScript::ResetRuntimeState() {

		m_PlayerDead = false;
		m_PlayerPosition = { 0.0f, 0.0f, 0.0f };
		m_OrbiterRadius = std::clamp(m_OrbiterRadius, m_OrbiterMinRadius, m_OrbiterMaxRadius);
		m_OrbiterRadialVelocity = 0.0f;
		for (int enemyIndex = 0; enemyIndex < OrbitMaxEnemyCount; enemyIndex++) {
			ResetEnemy(enemyIndex);
		}

	}

	void OrbitGameControllerScript::UpdatePlayer(float seconds) {

		glm::vec3 moveDirection{ 0.0f };
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_A)) { moveDirection.x -= 1.0f; }
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_D)) { moveDirection.x += 1.0f; }
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_W)) { moveDirection.y += 1.0f; }
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_S)) { moveDirection.y -= 1.0f; }

		if (glm::dot(moveDirection, moveDirection) > 0.0f) {
			moveDirection = glm::normalize(moveDirection);
			m_PlayerPosition += moveDirection * m_PlayerSpeed * seconds;
			m_PlayerPosition = ClampToArena(m_PlayerPosition, m_ArenaHalfSize);
		}

	}

	void OrbitGameControllerScript::UpdateOrbiter(float seconds) {

		bool spacePressed = Ayin::Input::IsKeyPressed(AYIN_KEY_SPACE);
		m_OrbiterRadialVelocity = (spacePressed ? m_OrbiterRadiusExpandSpeed : 0.0f) - m_OrbiterRadiusDecaySpeed;
		m_OrbiterRadius = std::clamp(m_OrbiterRadius + m_OrbiterRadialVelocity * seconds, m_OrbiterMinRadius, m_OrbiterMaxRadius);
		float radiusForSpeed = std::max(m_OrbiterRadius, 0.001f);
		float speedScale = std::sqrt(std::max(m_OrbiterMinRadius, m_OrbiterMaxRadius) / radiusForSpeed);
		m_OrbiterAngle += m_OrbiterAngularVelocity * speedScale * seconds;
		m_OrbiterSelfRotation = std::fmod(m_OrbiterSelfRotation + (160.0f + std::abs(m_OrbiterAngularVelocity) * 55.0f) * seconds, 360.0f);
		if (m_OrbiterAngle > s_TwoPi || m_OrbiterAngle < -s_TwoPi) {
			m_OrbiterAngle = std::fmod(m_OrbiterAngle, s_TwoPi);
		}
	}

	void OrbitGameControllerScript::UpdateEnemies(float seconds, const glm::vec3& playerPosition) {

		int enemyCount = std::clamp(m_TargetEnemyCount, 0, OrbitMaxEnemyCount);
		for (int enemyIndex = 0; enemyIndex < enemyCount; enemyIndex++) {
			glm::vec3 chaseDirection = playerPosition - m_EnemyPositions[enemyIndex];
			chaseDirection.z = 0.0f;
			if (glm::dot(chaseDirection, chaseDirection) > 0.0001f) {
				m_EnemyPositions[enemyIndex] += glm::normalize(chaseDirection) * m_EnemySpeed * seconds;
				m_EnemyPositions[enemyIndex] = ClampToArena(m_EnemyPositions[enemyIndex], m_ArenaHalfSize);
			}
		}

	}

	void OrbitGameControllerScript::ResolveCollisions(const glm::vec3& playerPosition, float seconds) {

		glm::vec3 orbiterPosition = GetOrbiterPosition(playerPosition);
		if (IsCircleOverlap(playerPosition, s_PlayerCollisionRadius, orbiterPosition, s_OrbiterCollisionRadius)) {
			m_PlayerDead = true;
		}

		int enemyCount = std::clamp(m_TargetEnemyCount, 0, OrbitMaxEnemyCount);
		for (int enemyIndex = 0; enemyIndex < enemyCount; enemyIndex++) {
			if (IsCircleOverlap(orbiterPosition, s_OrbiterCollisionRadius, m_EnemyPositions[enemyIndex], s_EnemyCollisionRadius)) {
				m_EnemyHealth[enemyIndex] -= m_EnemyDamagePerSecond * seconds;
				if (m_EnemyHealth[enemyIndex] <= 0.0f) {
					ResetEnemy(enemyIndex);
				}
			}

			if (IsCircleOverlap(playerPosition, s_PlayerCollisionRadius, m_EnemyPositions[enemyIndex], s_EnemyCollisionRadius)) {
				m_PlayerDead = true;
			}
		}

		if (m_PlayerDead) {
			m_PlayerPosition = { 0.0f, 0.0f, 0.0f };
			m_OrbiterRadius = 2.1f;
			m_OrbiterAngle = 0.0f;
			m_OrbiterRadialVelocity = 0.0f;
			for (int enemyIndex = 0; enemyIndex < OrbitMaxEnemyCount; enemyIndex++) {
				ResetEnemy(enemyIndex);
			}
			m_PlayerDead = false;
		}

	}

	void OrbitGameControllerScript::DrawOrbitScene() {

		auto&& [transform, camera] = GetComponents<Ayin::TransformComponent, Ayin::CameraComponent>();
		camera.Camera.SetViewMatrix(transform.Position, transform.Rotation);
		Ayin::Renderer2D::BeginScene(camera.Camera);
		Ayin::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.2f }, { 0.0f, 0.0f, 0.0f }, { m_ArenaHalfSize * 2.0f, m_ArenaHalfSize * 2.0f, 1.0f }, { 0.03f, 0.035f, 0.055f, 1.0f });

		glm::vec3 playerPosition = m_PlayerPosition;
		Ayin::Renderer2D::DrawQuad(playerPosition, { 0.0f, 0.0f, 0.0f }, { 0.72f, 0.72f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
		Ayin::Renderer2D::DrawQuad(GetOrbiterPosition(playerPosition), { 0.0f, 0.0f, m_OrbiterSelfRotation }, { 0.46f, 0.46f, 1.0f }, { 0.08f, 0.48f, 1.0f, 1.0f });

		int enemyCount = std::clamp(m_TargetEnemyCount, 0, OrbitMaxEnemyCount);
		for (int enemyIndex = 0; enemyIndex < enemyCount; enemyIndex++) {
			float alpha = std::clamp(m_EnemyHealth[enemyIndex] / std::max(0.1f, m_EnemyMaxHealth), 0.12f, 1.0f);
			Ayin::Renderer2D::DrawQuad(m_EnemyPositions[enemyIndex], { 0.0f, 0.0f, 0.0f }, { 0.62f, 0.62f, 1.0f }, { 1.0f, 0.12f, 0.08f, alpha });
		}

		Ayin::Renderer2D::EndScene();

	}

	void OrbitGameControllerScript::ResetEnemy(int index) {

		m_EnemyPositions[index] = GetEnemySpawnPosition(index);
		m_EnemyHealth[index] = std::max(0.1f, m_EnemyMaxHealth);

	}

	glm::vec3 OrbitGameControllerScript::GetEnemySpawnPosition(int index) const {

		switch (index % 5) {
		case 0: return { -3.8f, 2.4f, 0.0f };
		case 1: return { 3.2f, 2.8f, 0.0f };
		case 2: return { -3.1f, -2.9f, 0.0f };
		case 3: return { 3.9f, -1.8f, 0.0f };
		default: return { 0.0f, 4.6f, 0.0f };
		}

	}

	glm::vec3 OrbitGameControllerScript::GetOrbiterPosition(const glm::vec3& playerPosition) const {

		return playerPosition + glm::vec3{ std::cos(m_OrbiterAngle) * m_OrbiterRadius, std::sin(m_OrbiterAngle) * m_OrbiterRadius, 0.1f };

	}

}
