#include <AyinPch.h>

#include "OrbitLayer.h"

#include "OrbitGame.h"

OrbitLayer::OrbitLayer()
	: Ayin::Layer("OrbitGameLayer")
{}

void OrbitLayer::OnAttach() {

	AYIN_PROFILE_FUNCTION();
	ResetScene();

}

void OrbitLayer::OnUpdate(Ayin::Timestep deltaTime) {

	AYIN_PROFILE_FUNCTION();

	Ayin::RenderCommand::SetClearColor(m_ClearColor);
	Ayin::RenderCommand::Clear();
	Ayin::Renderer2D::ResetStatistics();

	if (m_Scene) {
		m_Scene->OnUpdateRuntime(deltaTime);
	}

}

void OrbitLayer::OnImGuiRender() {

	Ayin::Renderer2D::Statistics statistics = Ayin::Renderer2D::GetStatistics();

	ImGui::Begin("Orbit Combat");
	ImGui::TextUnformatted("WASD: move player");
	ImGui::TextUnformatted("Space: pull the blue orbiter inward");
	ImGui::TextUnformatted("Release Space: let the orbiter drift outward");
	ImGui::Text("Draw Calls: %d", statistics.DrawCalls);
	ImGui::Text("Quads: %d", statistics.QuadCount);
	ImGui::ColorEdit4("Clear Color", &m_ClearColor.x);
	if (ImGui::Button("Reset Scene")) {
		ResetScene();
	}
	ImGui::End();

}

void OrbitLayer::OnEvent(Ayin::Event& event) {

	Ayin::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Ayin::WindowResizeEvent>(AYIN_BIND_EVENT_FUN(OrbitLayer::OnWindowResize));

}

void OrbitLayer::ResetScene() {

	OrbitGame::RegisterScripts();
	m_Scene = OrbitGame::CreateOrbitCombatScene();

	const Ayin::Window& window = Ayin::Application::Get().GetWindow();
	m_Scene->OnViewportResize(window.GetWidth(), window.GetHeight());

}

bool OrbitLayer::OnWindowResize(Ayin::WindowResizeEvent& event) {

	if (m_Scene) {
		m_Scene->OnViewportResize(event.GetWidth(), event.GetHeight());
	}

	return false;

}
