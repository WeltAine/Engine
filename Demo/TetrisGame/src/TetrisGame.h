#pragma once

#include <Ayin.h>

#include <glm/glm.hpp>

#include <array>
#include <string>
#include <vector>

namespace TetrisGame {

	inline constexpr const char* MainCameraName = "MainCamera";
	inline constexpr const char* ControllerName = "TetrisController";
	inline constexpr const char* TetrisScenePath = "assets/scenes/tetris.json";

	inline constexpr int BoardWidth = 10;
	inline constexpr int BoardHeight = 20;
	inline constexpr float CellSize = 0.6f;
	inline constexpr float BoardOffsetX = -3.0f;
	inline constexpr float BoardOffsetY = -6.0f;
	inline constexpr float PreviewPanelCenterX = 6.2f;
	inline constexpr float PreviewPanelCenterY = 1.0f;
	inline constexpr float PreviewPanelSize = CellSize * 5.0f;
	inline constexpr float SceneCameraHeight = 14.0f;

	enum class PieceType : int {
		None = 0,
		I = 1,
		O = 2,
		T = 3,
		S = 4,
		Z = 5,
		J = 6,
		L = 7
	};

	enum class GameState : int {
		Playing = 0,
		Paused = 1,
		GameOver = 2
	};

	using PieceShape = std::array<std::array<int, 4>, 4>;
	using PieceRotation = std::array<PieceShape, 4>;

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
		bool CanMove(int pieceType, int rotation, int x, int y);
		void PlacePiece();
		int ClearLines();
		void ApplyLineClearScore(int linesCleared);
		void LockPieceAndSpawnNext();
		void RotatePiece();
		void MovePiece(int dx, int dy);
		void HardDrop();
		void UpdateBoard();
		void DrawBoard();
		void DrawPiece(int pieceType, int rotation, int x, int y, float alpha = 1.0f);
		void DrawGhost();
		void DrawNextPiece();
		void ResetGame();

		glm::vec4 GetPieceColor(int pieceType);

	private:
		std::array<std::array<int, BoardWidth>, BoardHeight> m_Board{};
		int m_CurrentPiece = 0;
		int m_CurrentRotation = 0;
		int m_CurrentX = 0;
		int m_CurrentY = 0;
		int m_NextPiece = 0;

		int m_Score = 0;
		int m_Level = 1;
		int m_LinesCleared = 0;
		GameState m_GameState = GameState::Playing;

		float m_DropTimer = 0.0f;
		float m_DropInterval = 1.0f;
		float m_InputTimer = 0.0f;
		float m_InputDelay = 0.1f;

		std::array<PieceRotation, 8> m_Pieces{};
		std::array<int, 7> m_PieceBag{};
		int m_PieceBagIndex = 7;

		bool m_GameOverShown = false;

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

	Ayin::Ref<Ayin::Scene> CreateTetrisScene();
	Ayin::Ref<Ayin::Scene> LoadTetrisScene();
	void PopulateTetrisScene(const Ayin::Ref<Ayin::Scene>& scene);

}
