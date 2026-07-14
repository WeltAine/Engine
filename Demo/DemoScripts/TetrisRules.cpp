#include <AyinPch.h>

#include "DemoScripts.h"

#include <algorithm>

namespace DemoScripts {

	bool TetrisGameControllerScript::CanMove(int pieceType, int rotation, int x, int y) const {

		if (pieceType <= 0 || pieceType >= static_cast<int>(m_Pieces.size())) {
			return false;
		}

		const auto& shape = m_Pieces[pieceType][rotation];
		for (int py = 0; py < 4; py++) {
			for (int px = 0; px < 4; px++) {
				if (shape[py][px] == 0) {
					continue;
				}
				int boardX = x + px;
				int boardY = y + py;
				if (boardX < 0 || boardX >= TetrisBoardWidth || boardY < 0) {
					return false;
				}
				if (boardY < TetrisBoardHeight && m_Board[boardY][boardX] != 0) {
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
				if (shape[py][px] == 0) {
					continue;
				}
				int boardX = m_CurrentX + px;
				int boardY = m_CurrentY + py;
				if (boardX >= 0 && boardX < TetrisBoardWidth && boardY >= 0 && boardY < TetrisBoardHeight) {
					m_Board[boardY][boardX] = m_CurrentPiece;
				}
			}
		}

	}

	int TetrisGameControllerScript::ClearLines() {

		int linesCleared = 0;
		for (int y = TetrisBoardHeight - 1; y >= 0; y--) {
			bool full = true;
			for (int x = 0; x < TetrisBoardWidth; x++) {
				if (m_Board[y][x] == 0) {
					full = false;
					break;
				}
			}
			if (!full) {
				continue;
			}
			linesCleared++;
			for (int row = y; row < TetrisBoardHeight - 1; row++) {
				m_Board[row] = m_Board[row + 1];
			}
			m_Board[TetrisBoardHeight - 1] = {};
			y++;
		}
		return linesCleared;

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
			m_GameState = TetrisGameState::GameOver;
		}

	}

	void TetrisGameControllerScript::RotatePiece() {

		int newRotation = (m_CurrentRotation + 1) % 4;
		if (CanMove(m_CurrentPiece, newRotation, m_CurrentX, m_CurrentY)) {
			m_CurrentRotation = newRotation;
		} else if (CanMove(m_CurrentPiece, newRotation, m_CurrentX - 1, m_CurrentY)) {
			m_CurrentRotation = newRotation;
			m_CurrentX--;
		} else if (CanMove(m_CurrentPiece, newRotation, m_CurrentX + 1, m_CurrentY)) {
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

}
