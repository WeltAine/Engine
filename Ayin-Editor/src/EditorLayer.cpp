#include <AyinPch.h>

#include "EditorLayer.h"



static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);//颜色


EditorLayer::EditorLayer()
	:Ayin::Layer("SandBox2DLayer"), m_CameraController(Ayin::CameraProp{ .Type = Ayin::Camera::CameraType::Orthogonal })
{
};


void EditorLayer::OnAttach() {

	AYIN_PROFILE_FUNCTION();

	m_Texture = Ayin::Texture2D::Create("O:/CppProgram/Ayin/assets/textures/blendTexture.png");

	Ayin::FramebufferSpecification specification{ .Size{1280, 720}, .Samples = 1 };
	m_Framebuffer = Ayin::Framebuffer::Create(specification);
	m_ViewportSize.x = 1280;
	m_ViewportSize.y = 720;


	// 设置场景
	{
		Ayin::Entity entity = m_ActiveScene.CreateEntity();
		auto& transform = entity.GetComponents<Ayin::TransformComponent>();
		transform = Ayin::TransformComponent{ glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 60.0f }, glm::vec3{ 1.0f, 1.0f, 1.0f } };
		auto& sprite = entity.AddComponent<Ayin::SpriteRendererComponent>();
		sprite.Color = { glm::vec4{ 0.8f, 0.2f, 0.5f, 0.5f } };
	}

	{
		m_TextureEntity = m_ActiveScene.CreateEntity("TextureEntity");
		auto& transform = m_TextureEntity.GetComponents<Ayin::TransformComponent>();
		transform = Ayin::TransformComponent{ glm::vec3{ 0.0f, 0.0f, -5.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 1.0f, 1.0f, 1.0f } };
		m_TextureEntity.AddComponent<Ayin::SpriteRendererComponent>().Texture2D = m_Texture;
	}

	for (float y = -5.0f; y < 5.0f; y += 0.5f)
	{
		for (float x = -5.0f; x < 5.0f; x += 0.5f)
		{
			std::string entityName = fmt::format("Entity_{:.1f}_{:.1f}", x, y);
			Ayin::Entity entity = m_ActiveScene.CreateEntity(entityName);
			auto& transform = entity.GetComponents<Ayin::TransformComponent>();
			transform = Ayin::TransformComponent{ glm::vec3{ x, y, -10.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 1.0f, 1.0f, 1.0f } };
			auto& sprite = entity.AddComponent<Ayin::SpriteRendererComponent>();
			sprite.Color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
		}
	}

	m_SceneCamera = m_ActiveScene.CreateEntity("MainCamera");
	m_SceneCamera.AddComponent<Ayin::CameraComponent>(Ayin::CameraProp{ .Type{Ayin::Camera::CameraType::Orthogonal} });

};
void EditorLayer::OnDetach() { AYIN_PROFILE_FUNCTION(); };

void EditorLayer::OnUpdate(Ayin::Timestep deltaTime) {

	AYIN_PROFILE_FUNCTION();

	//? 关于viewpoint在缩放时（主窗口不缩放），的黑屏问题
	//! 原因在于原先是在OnImGuiRender中改变帧缓冲大小
	//! 这导致帧缓冲的改变在渲染指令之后，OnUpdate中渲染的画面丢失
	//! 所以将帧缓冲改变提到渲染指令之前就好了
	//ToDo 顺带一提，ImGui窗口缩放时不会阻塞更新循环
	if (m_ViewportSize != m_Framebuffer->GetSpecification().Size && m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f)		
	{
		m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		Ayin::WindowResizeEvent resizeEvent{ (unsigned int)m_ViewportSize.x, (unsigned int)m_ViewportSize.y };
		m_CameraController.OnEvent(resizeEvent);

		Ayin::Renderer::OnWindowResize(m_ViewportSize.x, m_ViewportSize.y);
		//! Application::OnWindowsResize()中的调整并没有删去
		//! 我们已知，两帧之间处理事件，且处理的是上一帧的，指令-》ImGui-》事件-》输出画面-》。。。
		//! 事件的处理往往落后于事件的发生，ImGui中的事件处理要等到NewFrame时，也就是在Layer的OnUpdate和Layer的OnImGuiRender之间
		//! 而Application中的会在glfwPollEvents();时立刻处理，而ImGui此刻只是放入消息队列，等待NewFrame
		//! 所以同一个事件Application::OnWindowsResize()会先于ImGui::NewFrame触发，先于Ayin::Renderer::OnWindowResize触发
	}


	static float rotation = 0;
	rotation += deltaTime * 50.0f;

	{
		AYIN_PROFILE_SCOPE("Renderer Draw");


		m_Framebuffer->Bind();

		Ayin::RenderCommand::Clear();

		Ayin::Renderer2D::ResetStatistics();

		Ayin::Renderer2D::BeginScene(m_CameraController.GetCamera());


		Ayin::Renderer2D::EndScene();

		Ayin::Renderer2D::BeginScene(m_CameraController.GetCamera());

		m_TextureEntity.GetComponents<Ayin::TransformComponent>().Rotation = glm::vec3{ 0.0f, 0.0f, rotation };

		m_ActiveScene.OnUpdate(deltaTime);

		Ayin::Renderer2D::EndScene();

		Ayin::RenderCommand::SetClearColor({ clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w });

		m_Framebuffer->UnBind();

	}
};


void EditorLayer::OnImGuiRender() {


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

	if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered())		//聚焦且鼠标位于窗口上
		m_CameraController.OnUpdate(Ayin::Time::GetFrameInterval());


	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

	m_ViewTexture = m_Framebuffer->GetColorAttachment();
	ImGui::Image((void*)(long long int)(uint32_t)(*m_ViewTexture), ImVec2{ (float)m_ViewportSize.x, (float)m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

	ImGui::End();


	ImGui::Begin("Renderer2D Statistics");
	ImGui::Text("Draw Calls: %d", statistics.DrawCalls);
	ImGui::Text("TotalQuadCount: %d", statistics.QuadCount);

	ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

	ImGui::End();

};

void EditorLayer::OnEvent(Ayin::Event& event) {};
