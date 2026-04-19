#include <AyinPch.h>

#include "SandBox2D.h"


SandBox2D::SandBox2D()
	:Ayin::Layer("SandBox2DLayer"), m_CamreaController(Ayin::CameraProp{.Type = Ayin::Camera::CameraType::Orthogonal})
{};


void SandBox2D::OnAttach() {

	AYIN_PROFILE_FUNCTION();

	m_Texture = Ayin::Texture2D::Create("O:/CppProgram/Ayin/assets/textures/blendTexture.png");

};
void SandBox2D::OnDetach() {

	AYIN_PROFILE_FUNCTION();

};

void SandBox2D::OnUpdate(Ayin::Timestep deltaTime) {

	AYIN_PROFILE_FUNCTION();

	m_CamreaController.OnUpdate(deltaTime);

	{
		AYIN_PROFILE_SCOPE("Renderer Prep");
		Ayin::RenderCommand::Clear();
	}

	{
		AYIN_PROFILE_SCOPE("Renderer Draw");
		Ayin::Renderer2D::BeginScene(m_CamreaController.GetCamera());

		Ayin::Renderer2D::DrawQuad(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{0.0f, 0.0f, 60.0f}, glm::vec3{ 1.0f, 1.0f, 1.0f });
		Ayin::Renderer2D::DrawQuad(glm::vec3{ 0.0f, 0.0f, -5.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 1.0f, 1.0f, 1.0f }, m_Texture);

		Ayin::Renderer2D::EndScene();
	}

};

void SandBox2D::OnImGuiRender() {
	
};

void SandBox2D::OnEvent(Ayin::Event& event) {
	m_CamreaController.OnEvent(event);
};
