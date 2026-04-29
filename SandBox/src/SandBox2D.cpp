#include <AyinPch.h>

#include "SandBox2D.h"



static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);//颜色


SandBox2D::SandBox2D()
	:Ayin::Layer("SandBox2DLayer"), m_CameraController(Ayin::CameraProp{.Type = Ayin::Camera::CameraType::Orthogonal})
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

	static float rotation = 0;
	rotation += deltaTime * 50.0f;


	{
		AYIN_PROFILE_SCOPE("Renderer Prep");
		Ayin::RenderCommand::Clear();
	}

    {
        AYIN_PROFILE_SCOPE("Renderer Draw");
    
	    Ayin::Renderer2D::ResetStatistics();

	    Ayin::Renderer2D::BeginScene(m_CameraController.GetCamera());

	    for (float y = -5.0f; y < 5.0f; y += 0.5f)
	    {
		    for (float x = -5.0f; x < 5.0f; x += 0.5f)
		    {
			    glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
			    Ayin::Renderer2D::DrawQuad(glm::vec3{x, y, -10.0f}, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 1.0f, 1.0f, 1.0f }, color);
		    }
	    }

		Ayin::Renderer2D::EndScene();
	


	    Ayin::Renderer2D::BeginScene(m_CameraController.GetCamera());

	    Ayin::Renderer2D::DrawQuad(glm::vec3{ 0.0f, 0.0f, -5.0f }, glm::vec3{ 0.0f, 0.0f, rotation }, glm::vec3{ 1.0f, 1.0f, 1.0f }, m_Texture, glm::vec2{2.0f, 2.0f});
	    Ayin::Renderer2D::DrawQuad(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 60.0f }, glm::vec3{ 1.0f, 1.0f, 1.0f }, glm::vec4{0.8f, 0.2f, 0.5f, 0.5f});

	    Ayin::Renderer2D::EndScene();

	    Ayin::RenderCommand::SetClearColor({ clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w });
    }
};


void SandBox2D::OnImGuiRender() {


	Ayin::Renderer2D::Statistics statistics = Ayin::Renderer2D::GetStatistics();

	ImGui::Begin("Renderer2D Statistics");
	ImGui::Text("Draw Calls: %d", statistics.DrawCalls);
	ImGui::Text("TotalQuadCount: %d", statistics.QuadCount);

	ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

	ImGui::End();

};

void SandBox2D::OnEvent(Ayin::Event& event) {
	m_CameraController.OnEvent(event);
};
