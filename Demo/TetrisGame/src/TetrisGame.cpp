#include <AyinPch.h>

#include "TetrisGame.h"

#include "Ayin/Scene/SceneSerializer.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>

namespace {

	std::mt19937& GetRandomEngine() {
		static std::mt19937 s_RandomEngine{ std::random_device{}() };
		return s_RandomEngine;
	}

}

namespace TetrisGame {

	void TetrisCameraScript::OnUpdate(Ayin::Timestep deltaTime) {
		AYIN_PROFILE_FUNCTION();
		(void)deltaTime;
		auto&& [transform, camera] = GetComponents<Ayin::TransformComponent, Ayin::CameraComponent>();
		camera.Camera.SetViewMatrix(transform.Position, transform.Rotation);
	}

	void TetrisGameControllerScript::OnCreate() {
		InitPieces();
		ResetGame();
	}

	void TetrisGameControllerScript::OnUpdate(Ayin::Timestep deltaTime) {
		AYIN_PROFILE_FUNCTION();

		float seconds = std::min(deltaTime.GetSeconds(), 0.05f);

		if (m_GameState == GameState::GameOver) {
			DrawBoard();
			return;
		}

		if (m_GameState == GameState::Paused) {
			DrawBoard();
			return;
		}

		if (Ayin::Input::IsKeyPressed(AYIN_KEY_P) && m_InputTimer <= 0.0f) {
			m_GameState = (m_GameState == GameState::Paused) ? GameState::Playing : GameState::Paused;
			m_InputTimer = m_InputDelay;
		}

		m_InputTimer -= seconds;
		m_DropTimer += seconds;

		if (m_InputTimer <= 0.0f) {
			if (Ayin::Input::IsKeyPressed(AYIN_KEY_LEFT)) {
				MovePiece(-1, 0);
				m_InputTimer = m_InputDelay;
			}
			else if (Ayin::Input::IsKeyPressed(AYIN_KEY_RIGHT)) {
				MovePiece(1, 0);
				m_InputTimer = m_InputDelay;
			}
			else if (Ayin::Input::IsKeyPressed(AYIN_KEY_DOWN)) {
				MovePiece(0, -1);
				m_InputTimer = m_InputDelay * 0.5f;
			}
			else if (Ayin::Input::IsKeyPressed(AYIN_KEY_UP)) {
				RotatePiece();
				m_InputTimer = m_InputDelay * 1.5f;
			}
			else if (Ayin::Input::IsKeyPressed(AYIN_KEY_SPACE)) {
				HardDrop();
				m_InputTimer = m_InputDelay * 2.0f;
			}
		}

		if (m_DropTimer >= m_DropInterval) {
			m_DropTimer = 0.0f;
			if (!CanMove(m_CurrentPiece, m_CurrentRotation, m_CurrentX, m_CurrentY - 1)) {
				LockPieceAndSpawnNext();
			}
			else {
				m_CurrentY--;
			}
		}

		DrawBoard();
	}

	void TetrisGameControllerScript::OnGui() {
		ImGui::SeparatorText("Tetris Game Controller");
		ImGui::Text("Score: %d", m_Score);
		ImGui::Text("Level: %d", m_Level);
		ImGui::Text("Lines: %d", m_LinesCleared);
		ImGui::Text("State: %s", m_GameState == GameState::Playing ? "Playing" :
			m_GameState == GameState::Paused ? "Paused" : "Game Over");
		ImGui::DragFloat("Drop Interval", &m_DropInterval, 0.01f, 0.05f, 2.0f);

		if (ImGui::Button("Reset Game")) {
			ResetGame();
		}
	}

	void TetrisGameControllerScript::InitPieces() {
		// I piece
		m_Pieces[1][0] = {{
			{0, 0, 0, 0},
			{1, 1, 1, 1},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[1][1] = {{
			{0, 0, 1, 0},
			{0, 0, 1, 0},
			{0, 0, 1, 0},
			{0, 0, 1, 0}
		}};
		m_Pieces[1][2] = {{
			{0, 0, 0, 0},
			{0, 0, 0, 0},
			{1, 1, 1, 1},
			{0, 0, 0, 0}
		}};
		m_Pieces[1][3] = {{
			{0, 1, 0, 0},
			{0, 1, 0, 0},
			{0, 1, 0, 0},
			{0, 1, 0, 0}
		}};

		// O piece
		m_Pieces[2][0] = {{
			{0, 1, 1, 0},
			{0, 1, 1, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[2][1] = {{
			{0, 1, 1, 0},
			{0, 1, 1, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[2][2] = {{
			{0, 1, 1, 0},
			{0, 1, 1, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[2][3] = {{
			{0, 1, 1, 0},
			{0, 1, 1, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		}};

		// T piece
		m_Pieces[3][0] = {{
			{0, 1, 0, 0},
			{1, 1, 1, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[3][1] = {{
			{0, 1, 0, 0},
			{0, 1, 1, 0},
			{0, 1, 0, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[3][2] = {{
			{0, 0, 0, 0},
			{1, 1, 1, 0},
			{0, 1, 0, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[3][3] = {{
			{0, 1, 0, 0},
			{1, 1, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 0, 0}
		}};

		// S piece
		m_Pieces[4][0] = {{
			{0, 1, 1, 0},
			{1, 1, 0, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[4][1] = {{
			{0, 1, 0, 0},
			{0, 1, 1, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[4][2] = {{
			{0, 0, 0, 0},
			{0, 1, 1, 0},
			{1, 1, 0, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[4][3] = {{
			{1, 0, 0, 0},
			{1, 1, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 0, 0}
		}};

		// Z piece
		m_Pieces[5][0] = {{
			{1, 1, 0, 0},
			{0, 1, 1, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[5][1] = {{
			{0, 0, 1, 0},
			{0, 1, 1, 0},
			{0, 1, 0, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[5][2] = {{
			{0, 0, 0, 0},
			{1, 1, 0, 0},
			{0, 1, 1, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[5][3] = {{
			{0, 1, 0, 0},
			{1, 1, 0, 0},
			{1, 0, 0, 0},
			{0, 0, 0, 0}
		}};

		// J piece
		m_Pieces[6][0] = {{
			{1, 0, 0, 0},
			{1, 1, 1, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[6][1] = {{
			{0, 1, 1, 0},
			{0, 1, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[6][2] = {{
			{0, 0, 0, 0},
			{1, 1, 1, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[6][3] = {{
			{0, 1, 0, 0},
			{0, 1, 0, 0},
			{1, 1, 0, 0},
			{0, 0, 0, 0}
		}};

		// L piece
		m_Pieces[7][0] = {{
			{0, 0, 1, 0},
			{1, 1, 1, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[7][1] = {{
			{0, 1, 0, 0},
			{0, 1, 0, 0},
			{0, 1, 1, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[7][2] = {{
			{0, 0, 0, 0},
			{1, 1, 1, 0},
			{1, 0, 0, 0},
			{0, 0, 0, 0}
		}};
		m_Pieces[7][3] = {{
			{1, 1, 0, 0},
			{0, 1, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 0, 0}
		}};

		RefillPieceBag();
	}

	void TetrisGameControllerScript::RefillPieceBag() {
		m_PieceBag = { 1, 2, 3, 4, 5, 6, 7 };
		std::shuffle(m_PieceBag.begin(), m_PieceBag.end(), GetRandomEngine());
		m_PieceBagIndex = 0;
	}

	void TetrisGameControllerScript::SpawnPiece() {
		if (m_NextPiece == 0) {
			m_NextPiece = RandomPiece();
		}

		m_CurrentPiece = m_NextPiece;
		m_CurrentRotation = 0;
		m_CurrentX = BoardWidth / 2 - 2;
		m_CurrentY = BoardHeight - 2;
		m_NextPiece = RandomPiece();
		m_DropTimer = 0.0f;
	}

	int TetrisGameControllerScript::RandomPiece() {
		if (m_PieceBagIndex >= static_cast<int>(m_PieceBag.size())) {
			RefillPieceBag();
		}

		return m_PieceBag[m_PieceBagIndex++];
	}

	bool TetrisGameControllerScript::CanMove(int pieceType, int rotation, int x, int y) {
		if (pieceType <= 0 || pieceType >= static_cast<int>(m_Pieces.size())) {
			return false;
		}

		const auto& shape = m_Pieces[pieceType][rotation];
		for (int py = 0; py < 4; py++) {
			for (int px = 0; px < 4; px++) {
				if (shape[py][px] == 0) continue;
				int boardX = x + px;
				int boardY = y + py;
				if (boardX < 0 || boardX >= BoardWidth || boardY < 0) {
					return false;
				}
				if (boardY >= BoardHeight) {
					continue;
				}
				if (m_Board[boardY][boardX] != 0) {
					return false;
				}
			}
		}
		return true;
	}

	void TetrisGameControllerScript::PlacePiece() {
		const auto& shape = m_Pieces[m_CurrentPiece][m_CurrentRotation];
		for (int py = 0; py < 4; py++) {
			for (int px = 0; px < 4; px++) {
				if (shape[py][px] == 0) continue;
				int boardX = m_CurrentX + px;
				int boardY = m_CurrentY + py;
				if (boardX >= 0 && boardX < BoardWidth && boardY >= 0 && boardY < BoardHeight) {
					m_Board[boardY][boardX] = m_CurrentPiece;
				}
			}
		}
	}

	int TetrisGameControllerScript::ClearLines() {
		int linesCleared = 0;
		for (int y = BoardHeight - 1; y >= 0; y--) {
			bool full = true;
			for (int x = 0; x < BoardWidth; x++) {
				if (m_Board[y][x] == 0) {
					full = false;
					break;
				}
			}
			if (full) {
				linesCleared++;
				for (int row = y; row < BoardHeight - 1; row++) {
					m_Board[row] = m_Board[row + 1];
				}
				m_Board[BoardHeight - 1] = {};
				y++;
			}
		}
		return linesCleared;
	}

	void TetrisGameControllerScript::UpdateBoard() {
	}

	void TetrisGameControllerScript::ApplyLineClearScore(int linesCleared) {
		if (linesCleared <= 0) {
			return;
		}

		m_LinesCleared += linesCleared;
		switch (linesCleared) {
		case 1: m_Score += 100 * m_Level; break;
		case 2: m_Score += 300 * m_Level; break;
		case 3: m_Score += 500 * m_Level; break;
		case 4: m_Score += 800 * m_Level; break;
		default: break;
		}

		m_Level = m_LinesCleared / 10 + 1;
		m_DropInterval = std::max(0.1f, 1.0f - (m_Level - 1) * 0.05f);
	}

	void TetrisGameControllerScript::LockPieceAndSpawnNext() {
		PlacePiece();
		ApplyLineClearScore(ClearLines());
		SpawnPiece();

		if (!CanMove(m_CurrentPiece, m_CurrentRotation, m_CurrentX, m_CurrentY)) {
			m_GameState = GameState::GameOver;
		}
	}

	void TetrisGameControllerScript::RotatePiece() {
		int newRotation = (m_CurrentRotation + 1) % 4;
		if (CanMove(m_CurrentPiece, newRotation, m_CurrentX, m_CurrentY)) {
			m_CurrentRotation = newRotation;
		}
		else if (CanMove(m_CurrentPiece, newRotation, m_CurrentX - 1, m_CurrentY)) {
			m_CurrentRotation = newRotation;
			m_CurrentX--;
		}
		else if (CanMove(m_CurrentPiece, newRotation, m_CurrentX + 1, m_CurrentY)) {
			m_CurrentRotation = newRotation;
			m_CurrentX++;
		}
	}

	void TetrisGameControllerScript::MovePiece(int dx, int dy) {
		if (CanMove(m_CurrentPiece, m_CurrentRotation, m_CurrentX + dx, m_CurrentY + dy)) {
			m_CurrentX += dx;
			m_CurrentY += dy;
		}
	}

	void TetrisGameControllerScript::HardDrop() {
		while (CanMove(m_CurrentPiece, m_CurrentRotation, m_CurrentX, m_CurrentY - 1)) {
			m_CurrentY--;
		}
		LockPieceAndSpawnNext();
	}

	glm::vec4 TetrisGameControllerScript::GetPieceColor(int pieceType) {
		switch (pieceType) {
		case 1: return {0.0f, 1.0f, 1.0f, 1.0f};   // I - Cyan
		case 2: return {1.0f, 1.0f, 0.0f, 1.0f};    // O - Yellow
		case 3: return {0.6f, 0.0f, 0.8f, 1.0f};    // T - Purple
		case 4: return {0.0f, 1.0f, 0.0f, 1.0f};    // S - Green
		case 5: return {1.0f, 0.0f, 0.0f, 1.0f};    // Z - Red
		case 6: return {0.0f, 0.0f, 1.0f, 1.0f};    // J - Blue
		case 7: return {1.0f, 0.5f, 0.0f, 1.0f};    // L - Orange
		default: return {0.5f, 0.5f, 0.5f, 1.0f};
		}
	}

	void TetrisGameControllerScript::DrawBoard() {
		Ayin::Scene* scene = GetScene();
		if (scene == nullptr) {
			return;
		}

		std::vector<Ayin::Entity> cameraEntities = scene->GetEntitiesByComponents<Ayin::CameraComponent>();
		if (cameraEntities.empty()) {
			return;
		}

		auto& camera = cameraEntities[0].GetComponents<Ayin::CameraComponent>();
		Ayin::Renderer2D::BeginScene(camera.Camera);

		float offsetX = BoardOffsetX;
		float offsetY = BoardOffsetY;

		Ayin::Renderer2D::DrawQuad(
			{offsetX + BoardWidth * CellSize / 2.0f, offsetY + BoardHeight * CellSize / 2.0f, 0.0f},
			{0.0f, 0.0f, 0.0f},
			{BoardWidth * CellSize + 0.1f, BoardHeight * CellSize + 0.1f, 1.0f},
			{0.1f, 0.1f, 0.1f, 1.0f}
		);

		for (int y = 0; y < BoardHeight; y++) {
			for (int x = 0; x < BoardWidth; x++) {
				float posX = offsetX + x * CellSize + CellSize / 2.0f;
				float posY = offsetY + y * CellSize + CellSize / 2.0f;
				if (m_Board[y][x] != 0) {
					glm::vec4 color = GetPieceColor(m_Board[y][x]);
					Ayin::Renderer2D::DrawQuad(
						{posX, posY, 0.1f},
						{0.0f, 0.0f, 0.0f},
						{CellSize * 0.95f, CellSize * 0.95f, 1.0f},
						color
					);
				}
				else {
					Ayin::Renderer2D::DrawQuad(
						{posX, posY, 0.1f},
						{0.0f, 0.0f, 0.0f},
						{CellSize * 0.95f, CellSize * 0.95f, 1.0f},
						{0.15f, 0.15f, 0.15f, 1.0f}
					);
				}
			}
		}

		if (m_GameState != GameState::GameOver) {
			DrawGhost();
			DrawPiece(m_CurrentPiece, m_CurrentRotation, m_CurrentX, m_CurrentY);
		}

		DrawNextPiece();

		Ayin::Renderer2D::EndScene();
	}

	void TetrisGameControllerScript::DrawPiece(int pieceType, int rotation, int x, int y, float alpha) {
		const auto& shape = m_Pieces[pieceType][rotation];
		glm::vec4 color = GetPieceColor(pieceType);
		color.a = alpha;
		for (int py = 0; py < 4; py++) {
			for (int px = 0; px < 4; px++) {
				if (shape[py][px] == 0) continue;
				int boardY = y + py;
				if (boardY < 0 || boardY >= BoardHeight) continue;
				float posX = BoardOffsetX + (x + px) * CellSize + CellSize / 2.0f;
				float posY = BoardOffsetY + boardY * CellSize + CellSize / 2.0f;
				Ayin::Renderer2D::DrawQuad(
					{posX, posY, 0.2f},
					{0.0f, 0.0f, 0.0f},
					{CellSize * 0.95f, CellSize * 0.95f, 1.0f},
					color
				);
			}
		}
	}

	void TetrisGameControllerScript::DrawGhost() {
		int ghostY = m_CurrentY;
		while (CanMove(m_CurrentPiece, m_CurrentRotation, m_CurrentX, ghostY - 1)) {
			ghostY--;
		}
		if (ghostY != m_CurrentY) {
			DrawPiece(m_CurrentPiece, m_CurrentRotation, m_CurrentX, ghostY, 0.3f);
		}
	}

	void TetrisGameControllerScript::DrawNextPiece() {
		if (m_NextPiece <= 0 || m_NextPiece >= static_cast<int>(m_Pieces.size())) {
			return;
		}

		float nextX = PreviewPanelCenterX - CellSize * 2.0f;
		float nextY = PreviewPanelCenterY + CellSize * 1.5f;

		Ayin::Renderer2D::DrawQuad(
			{PreviewPanelCenterX, PreviewPanelCenterY, 0.0f},
			{0.0f, 0.0f, 0.0f},
			{PreviewPanelSize, PreviewPanelSize, 1.0f},
			{0.1f, 0.1f, 0.1f, 0.8f}
		);

		const auto& shape = m_Pieces[m_NextPiece][0];
		glm::vec4 color = GetPieceColor(m_NextPiece);
		for (int py = 0; py < 4; py++) {
			for (int px = 0; px < 4; px++) {
				if (shape[py][px] == 0) continue;
				float posX = nextX + px * CellSize + CellSize / 2.0f;
				float posY = nextY - py * CellSize - CellSize / 2.0f;
				Ayin::Renderer2D::DrawQuad(
					{posX, posY, 0.2f},
					{0.0f, 0.0f, 0.0f},
					{CellSize * 0.9f, CellSize * 0.9f, 1.0f},
					color
				);
			}
		}
	}

	void TetrisGameControllerScript::ResetGame() {
		m_Board = {};
		m_Score = 0;
		m_Level = 1;
		m_LinesCleared = 0;
		m_GameState = GameState::Playing;
		m_DropTimer = 0.0f;
		m_DropInterval = 1.0f;
		m_InputTimer = 0.0f;
		m_GameOverShown = false;
		m_CurrentPiece = 0;
		m_CurrentRotation = 0;
		m_CurrentX = BoardWidth / 2 - 2;
		m_CurrentY = BoardHeight - 2;
		RefillPieceBag();
		m_NextPiece = RandomPiece();
		SpawnPiece();
	}

	Ayin::Ref<Ayin::Scene> CreateTetrisScene() {
		auto scene = Ayin::CreateRef<Ayin::Scene>();
		PopulateTetrisScene(scene);
		return scene;
	}

	Ayin::Ref<Ayin::Scene> LoadTetrisScene() {
		if (std::filesystem::exists(TetrisScenePath)) {
			std::ifstream input{ TetrisScenePath };
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
		return CreateTetrisScene();
	}

	void PopulateTetrisScene(const Ayin::Ref<Ayin::Scene>& scene) {
		if (!scene) return;

		scene->SetName("Tetris Game");

		Ayin::Entity mainCamera = scene->CreateEntity(MainCameraName);
		mainCamera.GetComponents<Ayin::TransformComponent>().Position = { 0.0f, 0.0f, 10.0f };
		mainCamera.AddComponent<Ayin::CameraComponent>(Ayin::CameraProp{
			.Type = Ayin::Camera::CameraType::Orthogonal,
			.Height = SceneCameraHeight,
			.AspectRatio = 16.0f / 9.0f,
			.NearPlaneDistance = 0.1f,
			.FarPlaneDistance = 100.0f
		});
		mainCamera.AddScript<TetrisCameraScript>();

		Ayin::Entity controller = scene->CreateEntity(ControllerName);
		controller.AddScript<TetrisGameControllerScript>();
	}

}
