#pragma once

#include <Ayin.h>

#include <glm/glm.hpp>

#include <array>
#include <string>

namespace DemoScripts {

	inline constexpr const char* TetrisCameraEntityName = "MainCamera";
	inline constexpr const char* TetrisControllerEntityName = "TetrisController";
	inline constexpr const char* OrbitCameraEntityName = "MainCamera";
	inline constexpr const char* OrbitControllerEntityName = "OrbitGameController";
	inline constexpr const char* TutorialControllerEntityName = "TutorialController";

	inline constexpr int TetrisBoardWidth = 10;
	inline constexpr int TetrisBoardHeight = 20;
	inline constexpr float TetrisCellSize = 0.6f;
	inline constexpr float TetrisBoardOffsetX = -3.0f;
	inline constexpr float TetrisBoardOffsetY = -6.0f;
	inline constexpr float TetrisPreviewPanelCenterX = 6.2f;
	inline constexpr float TetrisPreviewPanelCenterY = 1.0f;
	inline constexpr float TetrisPreviewPanelSize = TetrisCellSize * 5.0f;
	inline constexpr float TetrisSceneCameraHeight = 14.0f;
	inline constexpr int OrbitMaxEnemyCount = 8;

	enum class TetrisGameState : int {
		Playing = 0,
		Paused = 1,
		GameOver = 2
	};

	using TetrisPieceShape = std::array<std::array<int, 4>, 4>;
	using TetrisPieceRotation = std::array<TetrisPieceShape, 4>;

	struct OrbitEnemyComponent {

		float Health = 1.0f;
		float MaxHealth = 1.0f;

		OrbitEnemyComponent() = default;
		explicit OrbitEnemyComponent(float maxHealth)
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

	class TetrisCameraScript : public Ayin::ScriptableEntity {

	public:
		AYIN_CUSTOM_SCRIPT_TYPE(TetrisCameraScript);

		void OnUpdate(Ayin::Timestep deltaTime) override;

	};
	AYIN_SCRIPT(TetrisCameraScript);

	class TetrisGameControllerScript : public Ayin::ScriptableEntity {

	public:
		AYIN_CUSTOM_SCRIPT_TYPE(TetrisGameControllerScript);

		void OnCreate() override;
		void OnUpdate(Ayin::Timestep deltaTime) override;
		void OnGui() override;

	private:
		void InitPieces();
		void RefillPieceBag();
		int RandomPiece();
		void SpawnPiece();
		bool CanMove(int pieceType, int rotation, int x, int y) const;
		void PlacePiece();
		int ClearLines();
		void ApplyLineClearScore(int linesCleared);
		void LockPieceAndSpawnNext();
		void RotatePiece();
		void MovePiece(int dx, int dy);
		void HardDrop();
		void DrawBoard();
		void DrawPiece(int pieceType, int rotation, int x, int y, float alpha = 1.0f) const;
		void DrawGhost();
		void DrawNextPiece() const;
		void ResetGame();
		glm::vec4 GetPieceColor(int pieceType) const;

	private:
		std::array<std::array<int, TetrisBoardWidth>, TetrisBoardHeight> m_Board{};
		int m_CurrentPiece = 0;
		int m_CurrentRotation = 0;
		int m_CurrentX = 0;
		int m_CurrentY = 0;
		int m_NextPiece = 0;
		int m_Score = 0;
		int m_Level = 1;
		int m_LinesCleared = 0;
		TetrisGameState m_GameState = TetrisGameState::Playing;
		float m_DropTimer = 0.0f;
		float m_DropInterval = 1.0f;
		float m_InputTimer = 0.0f;
		float m_InputDelay = 0.1f;
		std::array<TetrisPieceRotation, 8> m_Pieces{};
		std::array<int, 7> m_PieceBag{};
		int m_PieceBagIndex = 7;

	public:
		struct glaze {
			using T = TetrisGameControllerScript;
			static constexpr auto value = glz::object(
				"Score", &T::m_Score,
				"Level", &T::m_Level,
				"LinesCleared", &T::m_LinesCleared,
				"DropInterval", &T::m_DropInterval
			);
		};

	};
	AYIN_SCRIPT(TetrisGameControllerScript);

	class OrbitCameraScript : public Ayin::ScriptableEntity {

	public:
		AYIN_CUSTOM_SCRIPT_TYPE(OrbitCameraScript);

		void OnUpdate(Ayin::Timestep deltaTime) override;

	};
	AYIN_SCRIPT(OrbitCameraScript);

	class OrbitGameControllerScript : public Ayin::ScriptableEntity {

	public:
		AYIN_CUSTOM_SCRIPT_TYPE(OrbitGameControllerScript);

		void OnCreate() override;
		void OnUpdate(Ayin::Timestep deltaTime) override;
		void OnGui() override;

	private:
		void ResetRuntimeState();
		void UpdatePlayer(float seconds);
		void UpdateOrbiter(float seconds);
		void UpdateEnemies(float seconds, const glm::vec3& playerPosition);
		void ResolveCollisions(const glm::vec3& playerPosition, float seconds);
		void DrawOrbitScene();
		void ResetEnemy(int index);
		glm::vec3 GetEnemySpawnPosition(int index) const;
		glm::vec3 GetOrbiterPosition(const glm::vec3& playerPosition) const;

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
		float m_OrbiterRadiusDecaySpeed = 0.22f;
		float m_OrbiterRadiusExpandSpeed = 0.95f;
		bool m_PlayerDead = false;
		glm::vec3 m_PlayerPosition{ 0.0f, 0.0f, 0.0f };
		std::array<glm::vec3, OrbitMaxEnemyCount> m_EnemyPositions{};
		std::array<float, OrbitMaxEnemyCount> m_EnemyHealth{};

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
				"OrbiterRadiusDecaySpeed", &T::m_OrbiterRadiusDecaySpeed,
				"OrbiterRadiusExpandSpeed", &T::m_OrbiterRadiusExpandSpeed,
				"PlayerDead", &T::m_PlayerDead
			);
		};

	};
	AYIN_SCRIPT(OrbitGameControllerScript);

	class TutorialSceneControllerScript : public Ayin::ScriptableEntity {

	public:
		AYIN_CUSTOM_SCRIPT_TYPE(TutorialSceneControllerScript);

		void OnUpdate(Ayin::Timestep deltaTime) override;
		void OnGui() override;

	private:
		float m_RotationSpeed = 35.0f;
		float m_ElapsedSeconds = 0.0f;

	public:
		struct glaze {
			using T = TutorialSceneControllerScript;
			static constexpr auto value = glz::object(
				"RotationSpeed", &T::m_RotationSpeed,
				"ElapsedSeconds", &T::m_ElapsedSeconds
			);
		};

	};
	AYIN_SCRIPT(TutorialSceneControllerScript);

}
