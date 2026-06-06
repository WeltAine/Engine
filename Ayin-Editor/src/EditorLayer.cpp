#include <AyinPch.h>

#include "EditorLayer.h"

#include "Ayin/Scene/SceneSerializer.h"

#include "Ayin/Utils/PlatformUtils.h"

#include "Ayin/Math/Math.h"

#include <glm/glm.hpp>


static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);//没帧初始颜色


EditorLayer::EditorLayer()
	:Ayin::Layer("SandBox2DLayer"), m_CameraController(Ayin::CameraProp{ .Type = Ayin::Camera::CameraType::Orthogonal })
{
};


void EditorLayer::OnAttach() {

	AYIN_PROFILE_FUNCTION();

	m_Texture = Ayin::Texture2D::Create("O:/CppProgram/Ayin/assets/textures/blendTexture.png");

	Ayin::FramebufferSpecification specification{ .Size{1280, 720}, .Samples = 1 };
	m_Framebuffer = Ayin::Framebuffer::Create(specification);
	m_SceneSize.x = 1280;
	m_SceneSize.y = 720;

	m_ActiveScene = Ayin::CreateRef<Ayin::Scene>();

	//x 设置场景
	//xfor (float y = -5.0f; y < 5.0f; y += 0.5f)
	//x{
	//x	for (float x = -5.0f; x < 5.0f; x += 0.5f)
	//x	{
	//x		std::string entityName = fmt::format("Entity_{:.1f}_{:.1f}", x, y);
	//x		Ayin::Entity entity = m_ActiveScene->CreateEntity(entityName);
	//x		auto& transform = entity.GetComponents<Ayin::TransformComponent>();
	//x		transform = Ayin::TransformComponent{ glm::vec3{ x, y, -10.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 1.0f, 1.0f, 1.0f } };
	//x		auto& sprite = entity.AddComponent<Ayin::SpriteRendererComponent>();
	//x		sprite.Color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
	//x	}
	//x}

	{
		//x m_TextureEntity = m_ActiveScene->CreateEntity("TextureEntity");
		auto&& entity = m_ActiveScene->CreateEntity("TextureEntity");
		//x auto& transform = m_TextureEntity.GetComponents<Ayin::TransformComponent>();
		//x transform = Ayin::TransformComponent{ glm::vec3{ 0.0f, 0.0f, -5.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 1.0f, 1.0f, 1.0f } };
		//x m_TextureEntity.AddComponent<Ayin::SpriteRendererComponent>().Texture2D = m_Texture;
		entity.AddComponent<Ayin::SpriteRendererComponent>().Texture2D = m_Texture;
	}


	{
		Ayin::Entity entity = m_ActiveScene->CreateEntity();
		auto& transform = entity.GetComponents<Ayin::TransformComponent>();
		transform = Ayin::TransformComponent{ glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 60.0f }, glm::vec3{ 1.0f, 1.0f, 1.0f } };
		auto& sprite = entity.AddComponent<Ayin::SpriteRendererComponent>();
		sprite.Color = { glm::vec4{ 0.8f, 0.2f, 0.5f, 0.5f } };
	}


	m_SceneCamera = m_ActiveScene->CreateEntity("MainCamera");
	m_SceneCamera.GetComponents<Ayin::TransformComponent>().Position.z = 10.0f;
	m_SceneCamera.AddComponent<Ayin::CameraComponent>(Ayin::CameraProp{ .Type{Ayin::Camera::CameraType::Perspective} });


	m_SceneCamera.AddComponent<Ayin::NativeScriptComponent>().Bind<CameraControllerScript>();

	m_SceneHierarchyPanel.SetContext(m_ActiveScene);

};
void EditorLayer::OnDetach() { AYIN_PROFILE_FUNCTION(); };

void EditorLayer::OnUpdate(Ayin::Timestep deltaTime) {

	AYIN_PROFILE_FUNCTION();

	// 窗内部口缩放
	//? 关于viewpoint在缩放时（主窗口不缩放），的黑屏问题
	//! 原因在于原先是在OnImGuiRender中改变帧缓冲大小
	//! 这导致帧缓冲的改变在渲染指令之后，OnUpdate中渲染的画面丢失
	//! 所以将帧缓冲改变提到渲染指令之前就好了
	//ToDo 顺带一提，ImGui窗口缩放时不会阻塞更新循环
	if (m_SceneSize != m_Framebuffer->GetSpecification().Size && m_SceneSize.x > 0.0f && m_SceneSize.y > 0.0f)
	{
		m_Framebuffer->Resize((uint32_t)m_SceneSize.x, (uint32_t)m_SceneSize.y);

		Ayin::WindowResizeEvent resizeEvent{ (unsigned int)m_SceneSize.x, (unsigned int)m_SceneSize.y };

		//相机比例调整
		m_CameraController.OnEvent(resizeEvent);
		//x m_SceneCamera.GetComponents<Ayin::CameraComponent>().Camera.SetCameraSize(m_ViewportSize.x, m_ViewportSize.y);
		m_ActiveScene->OnViewportResize(m_SceneSize.x, m_SceneSize.y);

		Ayin::Renderer::OnWindowResize(m_SceneSize.x, m_SceneSize.y);
		//! Application::OnWindowsResize()中的调整并没有删去
		//! 我们已知，两帧之间处理事件，且处理的是上一帧的，指令-》ImGui-》事件-》输出画面-》。。。
		//! 事件的处理往往落后于事件的发生，ImGui中的事件处理要等到NewFrame时，也就是在Layer的OnUpdate和Layer的OnImGuiRender之间
		//! 而Application中的会在glfwPollEvents();时立刻处理，而ImGui此刻只是放入消息队列，等待NewFrame
		//! 所以同一个事件Application::OnWindowsResize()会先于ImGui::NewFrame触发，先于Ayin::Renderer::OnWindowResize触发
	}


	//x static float rotation = 0;
	//x rotation += deltaTime * 50.0f;

	{
		AYIN_PROFILE_SCOPE("Renderer Draw");


		m_Framebuffer->Bind();

		Ayin::RenderCommand::Clear();

		Ayin::Renderer2D::ResetStatistics();



		//x m_TextureEntity.GetComponents<Ayin::TransformComponent>().Rotation = glm::vec3{ 0.0f, 0.0f, rotation };

		m_ActiveScene->OnUpdate(deltaTime);


		Ayin::RenderCommand::SetClearColor({ clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w });

		m_Framebuffer->UnBind();

	}
};


void EditorLayer::OnImGuiRender() {

	static int s_CameraModeSelection = 2; // 默认选中 Perspective (对应值 1)

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

	{
		ImGui::Begin("Viewport");

		if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered()) {}		//聚焦且鼠标位于窗口上
		//x m_CameraController.OnUpdate(Ayin::Time::GetFrameInterval());


		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();			//获取Viewport的可用绘制大小（也就是等于Image的实际绘制大小）

		// 显示场景
		float halfZoomLevel = m_SceneCamera.GetComponents<Ayin::CameraComponent>().Camera.GetCameraZoomLevel() * 0.5f;

		m_ViewTexture = m_Framebuffer->GetColorAttachment();
		ImGui::Image((void*)(long long int)(uint32_t)(*m_ViewTexture), viewportPanelSize, ImVec2{ 0.5f - halfZoomLevel, 0.5f + halfZoomLevel }, ImVec2{ 0.5f + halfZoomLevel, 0.5f - halfZoomLevel });
		//首先常规的纹理概念下，纹理左下角为00，右上角为11，openGL中也是如此
		//ImGui的坐标是左上角原点，窗口区域为第一象限，在它的纹理显示时也是这个坐标系来放置纹理，导致图片旋转180度
		//OpenGL中坐标是常见的数学布局
		//后两个参数就是用来更改ImGui左上角和右上角所使用的纹理uv

		// Gizmo数据准备（绘制位置和范围）
		ImVec2 sceneMin = ImGui::GetItemRectMin();					//获取Image的起始绘制位置
		ImVec2 sceneSize = ImGui::GetItemRectSize();				//获取Image的实际绘制大小

		// Gizmo
		// 每帧初始化ImGuizmo绘制列表和热区状态（原在ImGuiLayer::Begin()中调用，移至此处归属编辑器层）
		// SetNextWindowViewport：强制"gizmo"窗口留在主视口，防止ViewportsEnable模式下被提升为独立GLFW窗口导致黑色矩形bug
		ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
		ImGuizmo::BeginFrame();

		if(m_SceneHierarchyPanel.GetSelectedEntity())
		{
			// 相机
			Ayin::SceneCamera& camera = m_SceneCamera.GetComponents<Ayin::CameraComponent>().Camera;
			// 实体transform
			Ayin::TransformComponent& selectedEntityTransform = const_cast<Ayin::Entity&>(m_SceneHierarchyPanel.GetSelectedEntity()).GetComponents<Ayin::TransformComponent>();
			glm::mat4 transform = selectedEntityTransform;

			//ImGuizmo显示配置
			ImGuizmo::SetOrthographic((camera.GetCameraType() == Ayin::Camera::CameraType::Orthogonal) ? true : false);
			ImGuizmo::SetDrawlist();// 制到当前Viewport窗口，保证在Image之后叠加显示
			ImGuizmo::SetRect(sceneMin.x, sceneMin.y, sceneSize.x, sceneSize.y);//用于NDC

			ImGuizmo::Manipulate(
				glm::value_ptr(camera.GetViewMatrix()), glm::value_ptr(camera.GetProjectionMatrix()),
				m_GizmoOperation, m_GizmoMode,
				glm::value_ptr(transform), nullptr, nullptr, nullptr, nullptr
			);


			if (ImGuizmo::IsUsing()) {
				
				auto&&[positiong, rotation, scale] = Ayin::Math::DecomposeTransform(transform);

				selectedEntityTransform.Position = positiong;
				selectedEntityTransform.Rotation = glm::degrees(rotation);
				selectedEntityTransform.Scale = scale;

			}

		}

		DrawGiamoToolbarOverlay(sceneMin);

		m_SceneSize = { sceneSize.x, sceneSize.y };					//记录场景尺寸（Image实际绘制大小范围）（用于缩放时更新GPU资源大小）

		ImGui::End();
	}
	

	{
		ImGui::Begin("Renderer2D Statistics");
		ImGui::Text("Draw Calls: %d", statistics.DrawCalls);
		ImGui::Text("TotalQuadCount: %d", statistics.QuadCount);

		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		ImGui::End();
	}


	m_SceneHierarchyPanel.OnImGuiRender();

	m_PropertiesPanel.SetContext(m_SceneHierarchyPanel.GetSelectedEntity());
	m_PropertiesPanel.OnImGuiRender();



	// 序列化反序列化菜单
	{
	
		if (ImGui::BeginMainMenuBar()) {
			
			if (ImGui::BeginMenu("FILE")) {
			
				if (ImGui::MenuItem("SaveScene")) {
					SaveScene();
				}
				if (ImGui::MenuItem("OpenScene")) {
					OpenScene();
				}
				if (ImGui::MenuItem("CreateScene")) {
					NewScene();
				}

				ImGui::EndMenu();
			
			}
			
		
			ImGui::EndMainMenuBar();
		}

	}
};

void EditorLayer::OnEvent(Ayin::Event& event) {};


//快捷键
bool EditorLayer::OnKeyPressed(Ayin::KeyPressedEvent& keyPressedEvent) {

	bool isCtrl = Ayin::Input::IsKeyPressed(Ayin::KeyCode::Left_Control) && Ayin::Input::IsKeyPressed(Ayin::KeyCode::Right_Control);
	bool isShift = Ayin::Input::IsKeyPressed(Ayin::KeyCode::Left_Shift) && Ayin::Input::IsKeyPressed(Ayin::KeyCode::Right_Shift);

	switch (keyPressedEvent.GetKeyCode()) {
	
		//Ctrl + N
		case(Ayin::KeyCode::N): if (isCtrl) NewScene(); return true;
	
	}

};

void EditorLayer::DrawGiamoToolbarOverlay(ImVec2 leftTopPos) {
	
	leftTopPos.y += 8.0f;
	ImGui::SetCursorScreenPos(leftTopPos);//SetCursorPos是设置相对坐标

	// 目前就仅支持本地
	if (ImGui::RadioButton("T", m_GizmoOperation & ImGuizmo::TRANSLATE)) {
		m_GizmoOperation = (ImGuizmo::OPERATION)(m_GizmoOperation ^ ImGuizmo::TRANSLATE);
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("R", m_GizmoOperation & ImGuizmo::ROTATE)) {
		m_GizmoOperation = (ImGuizmo::OPERATION)(m_GizmoOperation ^ ImGuizmo::ROTATE);
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("S", m_GizmoOperation & ImGuizmo::SCALE)) {
		m_GizmoOperation = (ImGuizmo::OPERATION)(m_GizmoOperation ^ ImGuizmo::SCALE);
	}


};


//场景导入导出
void EditorLayer::OpenScene() {

	std::optional<std::string> filePath = Ayin::FileDialogs::OpenFile({ {"Scenen", "json"}}, nullptr);

	if (filePath) {
		m_ActiveScene = Ayin::CreateRef<Ayin::Scene>();


		Ayin::SceneSerializer sceneSerializer{ m_ActiveScene };
		sceneSerializer.Deserializer(*filePath);

		//{ //测试用
			m_SceneCamera = m_ActiveScene->GetEntitiesByComponents<Ayin::CameraComponent>()[0];
		//	auto& nsc = m_SceneCamera.AddComponent<Ayin::NativeScriptComponent>();
		//	if (!nsc.HasScript()) {
		//		nsc.Bind<CameraControllerScript>();
		//	}
		//}

		m_ActiveScene->OnViewportResize(m_SceneSize.x, m_SceneSize.y);

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

};

void EditorLayer::NewScene() {

	m_ActiveScene = Ayin::CreateRef<Ayin::Scene>();
	m_SceneHierarchyPanel.SetContext(m_ActiveScene);

	{ //测试用
		m_SceneCamera = m_ActiveScene->CreateEntity("MainCamera");
		m_SceneCamera.AddComponent<Ayin::CameraComponent>();
		m_SceneCamera.AddComponent<Ayin::NativeScriptComponent>().Bind<CameraControllerScript>();
	}

	m_ActiveScene->OnViewportResize(m_SceneSize.x, m_SceneSize.y);

};

void EditorLayer::SaveScene() {

	std::optional<std::string> filePath = Ayin::FileDialogs::SaveFile({ {"Scenen", "json"} }, "Scene");
	
	if (filePath) {
		Ayin::SceneSerializer sceneSerializer{ m_ActiveScene };
		sceneSerializer.Serializer(*filePath);
	} 

};
