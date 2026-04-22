#include <AyinPch.h>

#include "SandBox2D.h"


SandBox2D::SandBox2D()
	:Ayin::Layer("SandBox2DLayer"), m_CamreaController(Ayin::CameraProp{.Type = Ayin::Camera::CameraType::Orthogonal})
{};


void SandBox2D::OnAttach() {

	m_Texture = Ayin::Texture2D::Create("O:/CppProgram/Ayin/assets/textures/blendTexture.png");

};
void SandBox2D::OnDetach() {};

void SandBox2D::OnUpdate(Ayin::Timestep deltaTime) {

	m_CamreaController.OnUpdate(deltaTime);

	static float rotation = 0;
	rotation += deltaTime * 50.0f;

	Ayin::RenderCommand::Clear();

	Ayin::Renderer2D::BeginScene(m_CamreaController.GetCamera());

	for (float y = -5.0f; y < 5.0f; y += 0.5f)
	{
		for (float x = -5.0f; x < 5.0f; x += 0.5f)
		{
			glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
			Ayin::Renderer2D::DrawQuad(glm::vec3{x, y, -10.0f}, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 1.0f, 1.0f, 1.0f }, color);
		}
	}

	Ayin::Renderer2D::EndScene();


	Ayin::Renderer2D::BeginScene(m_CamreaController.GetCamera());

	Ayin::Renderer2D::DrawQuad(glm::vec3{ 0.0f, 0.0f, -5.0f }, glm::vec3{ 0.0f, 0.0f, rotation }, glm::vec3{ 1.0f, 1.0f, 1.0f }, m_Texture, glm::vec2{2.0f, 2.0f});
	Ayin::Renderer2D::DrawQuad(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 60.0f }, glm::vec3{ 1.0f, 1.0f, 1.0f }, glm::vec4{0.8f, 0.2f, 0.5f, 0.5f});

	Ayin::Renderer2D::EndScene();



};

void SandBox2D::OnImGuiRender() {
	
};

void SandBox2D::OnEvent(Ayin::Event& event) {
	m_CamreaController.OnEvent(event);
};
