#include <AyinPch.h>

#include "DemoScripts.h"

#include <algorithm>
#include <random>

namespace {

	std::mt19937& GetTetrisRandomEngine() {

		static std::mt19937 s_RandomEngine{ std::random_device{}() };
		return s_RandomEngine;

	}

}

namespace DemoScripts {

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
		m_InputTimer = std::max(0.0f, m_InputTimer - seconds);

		if (Ayin::Input::IsKeyPressed(AYIN_KEY_R) && m_InputTimer <= 0.0f) {
			ResetGame();
			m_InputTimer = m_InputDelay * 2.0f;
			DrawBoard();
			return;
		}

		if (Ayin::Input::IsKeyPressed(AYIN_KEY_P) && m_InputTimer <= 0.0f) {
			m_GameState = m_GameState == TetrisGameState::Paused ? TetrisGameState::Playing : TetrisGameState::Paused;
			m_InputTimer = m_InputDelay * 2.0f;
			DrawBoard();
			return;
		}

		if (m_GameState == TetrisGameState::Paused || m_GameState == TetrisGameState::GameOver) {
			DrawBoard();
			return;
		}

		m_DropTimer += seconds;
		if (m_InputTimer <= 0.0f) {
			if (Ayin::Input::IsKeyPressed(AYIN_KEY_LEFT)) {
				MovePiece(-1, 0);
				m_InputTimer = m_InputDelay;
			} else if (Ayin::Input::IsKeyPressed(AYIN_KEY_RIGHT)) {
				MovePiece(1, 0);
				m_InputTimer = m_InputDelay;
			} else if (Ayin::Input::IsKeyPressed(AYIN_KEY_DOWN)) {
				MovePiece(0, -1);
				m_InputTimer = m_InputDelay * 0.5f;
			} else if (Ayin::Input::IsKeyPressed(AYIN_KEY_UP)) {
				RotatePiece();
				m_InputTimer = m_InputDelay * 1.5f;
			} else if (Ayin::Input::IsKeyPressed(AYIN_KEY_SPACE)) {
				HardDrop();
				m_InputTimer = m_InputDelay * 2.0f;
			}
		}

		if (m_DropTimer >= m_DropInterval) {
			m_DropTimer = 0.0f;
			if (!CanMove(m_CurrentPiece, m_CurrentRotation, m_CurrentX, m_CurrentY - 1)) {
				LockPieceAndSpawnNext();
			} else {
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
		ImGui::Text("State: %s", m_GameState == TetrisGameState::Playing ? "Playing" : m_GameState == TetrisGameState::Paused ? "Paused" : "Game Over");
		ImGui::DragFloat("Drop Interval", &m_DropInterval, 0.01f, 0.05f, 2.0f);
		if (ImGui::Button("Reset Game")) {
			ResetGame();
		}

	}

	void TetrisGameControllerScript::RefillPieceBag() {

		m_PieceBag = { 1, 2, 3, 4, 5, 6, 7 };
		std::shuffle(m_PieceBag.begin(), m_PieceBag.end(), GetTetrisRandomEngine());
		m_PieceBagIndex = 0;

	}

	int TetrisGameControllerScript::RandomPiece() {

		if (m_PieceBagIndex >= static_cast<int>(m_PieceBag.size())) {
			RefillPieceBag();
		}
		return m_PieceBag[m_PieceBagIndex++];

	}

	void TetrisGameControllerScript::SpawnPiece() {

		if (m_NextPiece == 0) {
			m_NextPiece = RandomPiece();
		}
		m_CurrentPiece = m_NextPiece;
		m_CurrentRotation = 0;
		m_CurrentX = TetrisBoardWidth / 2 - 2;
		m_CurrentY = TetrisBoardHeight - 2;
		m_NextPiece = RandomPiece();
		m_DropTimer = 0.0f;

	}

	void TetrisGameControllerScript::ResetGame() {

		m_Board = {};
		m_Score = 0;
		m_Level = 1;
		m_LinesCleared = 0;
		m_GameState = TetrisGameState::Playing;
		m_DropTimer = 0.0f;
		m_DropInterval = 1.0f;
		m_InputTimer = 0.0f;
		m_CurrentPiece = 0;
		m_CurrentRotation = 0;
		m_CurrentX = TetrisBoardWidth / 2 - 2;
		m_CurrentY = TetrisBoardHeight - 2;
		RefillPieceBag();
		m_NextPiece = RandomPiece();
		SpawnPiece();

	}

}
