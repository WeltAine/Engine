#include <AyinPch.h>

#include "DemoScripts.h"

namespace DemoScripts {

	void TetrisGameControllerScript::DrawBoard() {

		auto&& [transform, camera] = GetComponents<Ayin::TransformComponent, Ayin::CameraComponent>();
		camera.Camera.SetViewMatrix(transform.Position, transform.Rotation);
		Ayin::Renderer2D::BeginScene(camera.Camera);

		Ayin::Renderer2D::DrawQuad(
			{ TetrisBoardOffsetX + TetrisBoardWidth * TetrisCellSize * 0.5f, TetrisBoardOffsetY + TetrisBoardHeight * TetrisCellSize * 0.5f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ TetrisBoardWidth * TetrisCellSize + 0.14f, TetrisBoardHeight * TetrisCellSize + 0.14f, 1.0f },
			{ 0.06f, 0.07f, 0.09f, 1.0f }
		);

		for (int y = 0; y < TetrisBoardHeight; y++) {
			for (int x = 0; x < TetrisBoardWidth; x++) {
				float posX = TetrisBoardOffsetX + x * TetrisCellSize + TetrisCellSize * 0.5f;
				float posY = TetrisBoardOffsetY + y * TetrisCellSize + TetrisCellSize * 0.5f;
				glm::vec4 color = m_Board[y][x] == 0 ? glm::vec4{ 0.13f, 0.15f, 0.18f, 1.0f } : GetPieceColor(m_Board[y][x]);
				Ayin::Renderer2D::DrawQuad(
					{ posX, posY, 0.1f },
					{ 0.0f, 0.0f, 0.0f },
					{ TetrisCellSize * 0.92f, TetrisCellSize * 0.92f, 1.0f },
					color
				);
			}
		}

		if (m_GameState != TetrisGameState::GameOver) {
			DrawGhost();
			DrawPiece(m_CurrentPiece, m_CurrentRotation, m_CurrentX, m_CurrentY);
		}

		DrawNextPiece();
		Ayin::Renderer2D::EndScene();

	}

	void TetrisGameControllerScript::DrawPiece(int pieceType, int rotation, int x, int y, float alpha) const {

		if (pieceType <= 0 || pieceType >= static_cast<int>(m_Pieces.size())) {
			return;
		}

		const auto& shape = m_Pieces[pieceType][rotation];
		glm::vec4 color = GetPieceColor(pieceType);
		color.w = alpha;
		for (int py = 0; py < 4; py++) {
			for (int px = 0; px < 4; px++) {
				if (shape[py][px] == 0) {
					continue;
				}
				int boardY = y + py;
				if (boardY < 0 || boardY >= TetrisBoardHeight) {
					continue;
				}

				float posX = TetrisBoardOffsetX + (x + px) * TetrisCellSize + TetrisCellSize * 0.5f;
				float posY = TetrisBoardOffsetY + boardY * TetrisCellSize + TetrisCellSize * 0.5f;
				Ayin::Renderer2D::DrawQuad(
					{ posX, posY, 0.2f },
					{ 0.0f, 0.0f, 0.0f },
					{ TetrisCellSize * 0.92f, TetrisCellSize * 0.92f, 1.0f },
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

	void TetrisGameControllerScript::DrawNextPiece() const {

		if (m_NextPiece <= 0 || m_NextPiece >= static_cast<int>(m_Pieces.size())) {
			return;
		}

		Ayin::Renderer2D::DrawQuad(
			{ TetrisPreviewPanelCenterX, TetrisPreviewPanelCenterY, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ TetrisPreviewPanelSize, TetrisPreviewPanelSize, 1.0f },
			{ 0.06f, 0.07f, 0.09f, 0.92f }
		);

		float nextX = TetrisPreviewPanelCenterX - TetrisCellSize * 2.0f;
		float nextY = TetrisPreviewPanelCenterY + TetrisCellSize * 1.5f;
		const auto& shape = m_Pieces[m_NextPiece][0];
		glm::vec4 color = GetPieceColor(m_NextPiece);
		for (int py = 0; py < 4; py++) {
			for (int px = 0; px < 4; px++) {
				if (shape[py][px] == 0) {
					continue;
				}
				float posX = nextX + px * TetrisCellSize + TetrisCellSize * 0.5f;
				float posY = nextY - py * TetrisCellSize - TetrisCellSize * 0.5f;
				Ayin::Renderer2D::DrawQuad(
					{ posX, posY, 0.2f },
					{ 0.0f, 0.0f, 0.0f },
					{ TetrisCellSize * 0.9f, TetrisCellSize * 0.9f, 1.0f },
					color
				);
			}
		}

	}

}
