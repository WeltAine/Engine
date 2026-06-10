#pragma once

#include <Ayin.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace OrbitGame {

	inline constexpr const char* MainCameraName = "MainCamera";
	inline constexpr const char* ControllerName = "OrbitGameController";
	inline constexpr const char* PlayerName = "Player";
	inline constexpr const char* OrbiterName = "OrbitOrbiter";
	inline constexpr const char* EnemyPrefix = "Enemy";
	inline constexpr const char* OrbitCombatScenePath = "assets/scenes/orbit_combat.json";

	// 敌人血量不是引擎通用概念，所以作为示例项目自己的组件放在这里。
	// SpriteRendererComponent 的 alpha 会由脚本根据 Health / MaxHealth 同步。
	struct OrbitEnemyComponent {

		float Health = 1.0f;
		float MaxHealth = 1.0f;

		OrbitEnemyComponent() = default;
		OrbitEnemyComponent(float maxHealth)
			: Health{ maxHealth }, MaxHealth{ maxHealth }
		{}

		static void OnGui(Ayin::Entity& entity);

		struct glaze {
			using T = OrbitEnemyComponent;
			static constexpr auto value = glz::object(
				"Health", &T::Health,
				"MaxHealth", &T::MaxHealth
			);
		};

	};
	AYIN_COMPONENT(OrbitEnemyComponent);
	AYIN_COMPONENTUI(OrbitEnemyComponent, OrbitEnemyComponent::OnGui);

	class OrbitCameraScript : public Ayin::ScriptableEntity {

	public:
		AYIN_CUSTOM_SCRIPT_TYPE(OrbitCameraScript);

		void OnUpdate(Ayin::Timestep deltaTime) override;

	};

	class OrbitGameControllerScript : public Ayin::ScriptableEntity {

	public:
		AYIN_CUSTOM_SCRIPT_TYPE(OrbitGameControllerScript);

		void OnCreate() override;
		void OnUpdate(Ayin::Timestep deltaTime) override;
		void OnGui() override;

	private:
		Ayin::Entity FindNamedEntity(const std::string& name);
		std::vector<Ayin::Entity> GetEnemies();
		glm::vec3 GenerateEnemySpawnPosition(const glm::vec3& playerPosition);
		void EnsureCoreEntities();
		void EnsureEnemyCount();
		void UpdatePlayer(float deltaTime);
		void UpdateOrbiter(float deltaTime);
		void UpdateEnemies(float deltaTime);
		void ResolveCollisions(float deltaTime);
		void RespawnEnemies();
		void KillPlayer();
		void ResetRuntimeState();

	private:
		float m_PlayerSpeed = 4.5f;
		float m_EnemySpeed = 1.5f;
		float m_EnemyMaxHealth = 1.0f;
		float m_EnemyDamagePerSecond = 0.75f;
		int m_TargetEnemyCount = 5;
		float m_ArenaHalfSize = 5.6f;

		float m_OrbiterRadius = 2.1f;
		float m_OrbiterMinRadius = 0.48f;
		float m_OrbiterMaxRadius = 3.4f;
		float m_OrbiterAngle = 0.0f;
		float m_OrbiterAngularVelocity = 1.4f;
		float m_OrbiterRadialVelocity = 0.0f;
		float m_OrbiterSelfRotation = 0.0f;
		float m_OrbiterGravity = 7.5f;
		float m_OrbiterPullStrength = 6.5f;
		float m_OrbiterReleaseStrength = 2.6f;
		float m_OrbiterOrbitCorrection = 1.35f;
		float m_OrbiterDrag = 0.04f;
		bool m_PlayerDead = false;

		glm::vec3 m_OrbiterVelocity{ 0.0f, 2.2f, 0.0f };

	public:

		struct glaze {
			using T = OrbitGameControllerScript;
			static constexpr auto value = glz::object(
				"PlayerSpeed", &T::m_PlayerSpeed,
				"EnemySpeed", &T::m_EnemySpeed,
				"EnemyMaxHealth", &T::m_EnemyMaxHealth,
				"EnemyDamagePerSecond", &T::m_EnemyDamagePerSecond,
				"TargetEnemyCount", &T::m_TargetEnemyCount,
				"ArenaHalfSize", &T::m_ArenaHalfSize,
				"OrbiterRadius", &T::m_OrbiterRadius,
				"OrbiterMinRadius", &T::m_OrbiterMinRadius,
				"OrbiterMaxRadius", &T::m_OrbiterMaxRadius,
				"OrbiterAngle", &T::m_OrbiterAngle,
				"OrbiterAngularVelocity", &T::m_OrbiterAngularVelocity,
				"OrbiterRadialVelocity", &T::m_OrbiterRadialVelocity,
				"OrbiterSelfRotation", &T::m_OrbiterSelfRotation,
				"OrbiterGravity", &T::m_OrbiterGravity,
				"OrbiterPullStrength", &T::m_OrbiterPullStrength,
				"OrbiterReleaseStrength", &T::m_OrbiterReleaseStrength,
				"OrbiterOrbitCorrection", &T::m_OrbiterOrbitCorrection,
				"OrbiterDrag", &T::m_OrbiterDrag,
				"OrbiterVelocity", &T::m_OrbiterVelocity,
				"PlayerDead", &T::m_PlayerDead
			);
		};

	};

	void RegisterScripts();

	Ayin::Ref<Ayin::Scene> CreateOrbitCombatScene();
	Ayin::Ref<Ayin::Scene> LoadOrbitCombatScene();
	void PopulateOrbitCombatScene(const Ayin::Ref<Ayin::Scene>& scene);
	Ayin::Entity CreateOrbitEnemy(Ayin::Scene& scene, const std::string& name, const glm::vec3& position, float maxHealth);

}
