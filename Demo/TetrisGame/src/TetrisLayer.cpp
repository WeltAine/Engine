#include <AyinPch.h>

#include "TetrisLayer.h"

#include "TetrisGame.h"

TetrisLayer::TetrisLayer()
	: Ayin::Layer("TetrisGameLayer")
{}

void TetrisLayer::OnAttach() {

	AYIN_PROFILE_FUNCTION();
	ResetScene();

}

void TetrisLayer::OnUpdate(Ayin::Timestep deltaTime) {

	AYIN_PROFILE_FUNCTION();

	Ayin::RenderCommand::SetClearColor(m_ClearColor);
	Ayin::RenderCommand::Clear();
	Ayin::Renderer2D::ResetStatistics();

	if (m_Scene) {
		m_Scene->OnUpdateRuntime(deltaTime);
	}

}

void TetrisLayer::OnImGuiRender() {

	Ayin::Renderer2D::Statistics statistics = Ayin::Renderer2D::GetStatistics();

	ImGui::Begin("Tetris");
	ImGui::TextUnformatted("Left/Right: Move piece");
	ImGui::TextUnformatted("Up: Rotate");
	ImGui::TextUnformatted("Down: Soft drop");
	ImGui::TextUnformatted("Space: Hard drop");
	ImGui::TextUnformatted("P: Pause/Resume");
	ImGui::Text("Draw Calls: %d", statistics.DrawCalls);
	ImGui::Text("Quads: %d", statistics.QuadCount);
	ImGui::ColorEdit4("Clear Color", &m_ClearColor.x);
	if (ImGui::Button("Reset Scene")) {
		ResetScene();
	}
	ImGui::End();

}

void TetrisLayer::OnEvent(Ayin::Event& event) {

	Ayin::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Ayin::WindowResizeEvent>(AYIN_BIND_EVENT_FUN(TetrisLayer::OnWindowResize));

}

void TetrisLayer::ResetScene() {

	m_Scene = TetrisGame::LoadTetrisScene();

	const Ayin::Window& window = Ayin::Application::Get().GetWindow();
	m_Scene->OnViewportResize(window.GetWidth(), window.GetHeight());

}

bool TetrisLayer::OnWindowResize(Ayin::WindowResizeEvent& event) {

	if (m_Scene) {
		m_Scene->OnViewportResize(event.GetWidth(), event.GetHeight());
	}

	return false;

}
