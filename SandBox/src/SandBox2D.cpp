#include <AyinPch.h>

#include "SandBox2D.h"



static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);//颜色


SandBox2D::SandBox2D()
	:Ayin::Layer("SandBox2DLayer"), m_CameraController(Ayin::CameraProp{.Type = Ayin::Camera::CameraType::Orthogonal})
{};


void SandBox2D::OnAttach() {

	m_Texture = Ayin::Texture2D::Create("O:/CppProgram/Ayin/assets/textures/blendTexture.png");

	Ayin::FramebufferSpecification specification{.Width = 1280 , .Height = 720, .Samples = 1};
	m_Framebuffer = Ayin::Framebuffer::Create(specification);
	m_ViewportSize.x = 1280;
	m_ViewportSize.y = 720;

};
void SandBox2D::OnDetach() {};

void SandBox2D::OnUpdate(Ayin::Timestep deltaTime) {

	m_CameraController.OnUpdate(deltaTime);

	static float rotation = 0;
	rotation += deltaTime * 50.0f;

	m_Framebuffer->Bind();

	Ayin::RenderCommand::Clear();

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

	m_Framebuffer->UnBind();
};


void SandBox2D::OnImGuiRender() {


	Ayin::Renderer2D::Statistics statistics = Ayin::Renderer2D::GetStatistics();


	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_MenuBar
		| ImGuiWindowFlags_NoDocking
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoNavFocus;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	bool dockSpaceOpen = true;
	ImGuiID dockspaceID = ImGui::GetID("##ui.dock_space");

	ImGui::DockSpaceOverViewport(dockspaceID, viewport, ImGuiDockNodeFlags_PassthruCentralNode);


	ImGui::Begin("Viewport");

	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	if (m_ViewportSize != *((glm::vec2*)&viewportPanelSize))
	{
		m_Framebuffer->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

		Ayin::WindowResizeEvent resizeEvent { (unsigned int)viewportPanelSize.x, (unsigned int)viewportPanelSize.y };
		m_CameraController.OnEvent(resizeEvent);
	}

	m_ViewTexture = m_Framebuffer->GetColorAttachment();
	ImGui::Image((void*)(long long int)(uint32_t)(*m_ViewTexture), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

	ImGui::End();


	ImGui::Begin("Renderer2D Statistics");
	ImGui::Text("Draw Calls: %d", statistics.DrawCalls);
	ImGui::Text("TotalQuadCount: %d", statistics.QuadCount);

	ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

	ImGui::End();





};

void SandBox2D::OnEvent(Ayin::Event& event) {
	m_CameraController.OnEvent(event);
};
