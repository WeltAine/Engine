#include <AyinPch.h>

#include "EditorLayer.h"

#include "Ayin/Scene/SceneSerializer.h"

#include "Ayin/Utils/PlatformUtils.h"

#include "Ayin/Math/Math.h"

#include <glm/glm.hpp>


static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);//没帧初始颜色


EditorLayer::EditorLayer()
	:Ayin::Layer("SandBox2DLayer"), m_EditorCamera{ 2.0f, 1280.0f / 720, 60.0f, 0.1f, 100.0f }
{
};


void EditorLayer::OnAttach() {

	AYIN_PROFILE_FUNCTION();

	const std::string testTexturePath = "assets/textures/blendTexture.png";
	m_Texture = Ayin::Texture2DLibrary::Load(testTexturePath);

	Ayin::FramebufferSpecification specification{ .Size{1280, 720}, .Samples = 1, .AttachmentsSpecification{Ayin::FramebufferAttachmentFormat::Color, Ayin::FramebufferAttachmentFormat::Red_Integer, Ayin::FramebufferAttachmentFormat::Depth_Stencil} };
	m_Framebuffer = Ayin::Framebuffer::Create(specification);
	m_SceneSize.x = 1280;
	m_SceneSize.y = 720;

	NewScene();

	//场景测试
	{
		auto&& entity = m_ActiveScene->CreateEntity("TextureEntity");
		entity.AddComponent<Ayin::SpriteRendererComponent>().SetTexture(testTexturePath);
	}


	{
		Ayin::Entity entity = m_ActiveScene->CreateEntity();
		auto& transform = entity.GetComponents<Ayin::TransformComponent>();
		transform = Ayin::TransformComponent{ glm::vec3{ 0.0f, 0.0f, 5.0f }, glm::vec3{ 0.0f, 0.0f, 60.0f }, glm::vec3{ 1.0f, 1.0f, 1.0f } };
		auto& sprite = entity.AddComponent<Ayin::SpriteRendererComponent>();
		sprite.Color = { glm::vec4{ 0.8f, 0.2f, 0.5f, 0.5f } };
	}
	//

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

		//编辑器相机比例调整
		m_EditorCamera.SetCameraAspect(float(m_SceneSize.x) / m_SceneSize.y);

		//调整场景中相机的比例
		m_ActiveScene->OnViewportResize(m_SceneSize.x, m_SceneSize.y);

		//跳针GPU的输出映射范围
		Ayin::Renderer::OnWindowResize(m_SceneSize.x, m_SceneSize.y);
		//! Application::OnWindowsResize()中的调整并没有删去
		//! 我们已知，两帧之间处理事件，且处理的是上一帧的，指令-》ImGui-》事件-》输出画面-》。。。
		//! 事件的处理往往落后于事件的发生，ImGui中的事件处理要等到NewFrame时，也就是在Layer的OnUpdate和Layer的OnImGuiRender之间
		//! 而Application中的会在glfwPollEvents();时立刻处理，而ImGui此刻只是放入消息队列，等待NewFrame
		//! 所以同一个事件Application::OnWindowsResize()会先于ImGui::NewFrame触发，先于Ayin::Renderer::OnWindowResize触发
	}

	{
		AYIN_PROFILE_SCOPE("Renderer Draw");


		m_Framebuffer->Bind();

		Ayin::RenderCommand::Clear();

		m_Framebuffer->ClearColorAttachment(1, Ayin::PixelR32I{-1});

		Ayin::Renderer2D::ResetStatistics();

		m_ActiveScene->OnUpdateEditor(deltaTime, m_EditorCamera);

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

		if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered())			//聚焦且鼠标位于窗口上
			m_EditorCamera.OnUpdate(Ayin::Time::GetFrameInterval());

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();			//获取Viewport的可用绘制大小（也就是等于Image的实际绘制大小）

		// 显示场景
		m_ViewTexture = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image((void*)(long long int)m_ViewTexture, viewportPanelSize, ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f });
		//首先常规的纹理概念下，纹理左下角为00，右上角为11，openGL中也是如此
		//ImGui的坐标是左上角原点，窗口区域为第一象限，在它的纹理显示时也是这个坐标系来放置纹理，导致图片旋转180度
		//OpenGL中坐标是常见的数学布局
		//后两个参数就是用来更改ImGui左上角和右下角所使用的纹理uv



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
			// 实体transform
			Ayin::TransformComponent& selectedEntityTransform = const_cast<Ayin::Entity&>(m_SceneHierarchyPanel.GetSelectedEntity()).GetComponents<Ayin::TransformComponent>();
			glm::mat4 transform = selectedEntityTransform;

			//ImGuizmo显示配置
			ImGuizmo::SetOrthographic((m_EditorCamera.GetCameraType() == Ayin::Camera::CameraType::Orthogonal) ? true : false);
			ImGuizmo::SetDrawlist();// 制到当前Viewport窗口，保证在Image之后叠加显示
			ImGuizmo::SetRect(sceneMin.x, sceneMin.y, sceneSize.x, sceneSize.y);//用于NDC
			ImGuizmo::SetGizmoSizeClipSpace((m_GizmoOperation & ImGuizmo::ROTATE) ? 0.16f : 0.1f);

			ImGuizmo::Manipulate(
				glm::value_ptr(m_EditorCamera.GetViewMatrix()), glm::value_ptr(m_EditorCamera.GetProjectionMatrix()),
				m_GizmoOperation, m_GizmoMode,
				glm::value_ptr(transform), nullptr, nullptr, nullptr, nullptr
			);


			if (ImGuizmo::IsUsing()) {
				
				auto&&[positiong, rotation, scale] = Ayin::Math::DecomposeTransform(transform);

				if (glm::all(glm::isfinite(positiong)) && glm::all(glm::isfinite(rotation)) && glm::all(glm::isfinite(scale))) {

					selectedEntityTransform.Position = positiong;
					selectedEntityTransform.Rotation = glm::degrees(rotation);
					//x selectedEntityTransform.Scale += (scale / selectedEntityTransform.Scale) - glm::vec3{ 1.0f, 1.0f, 1.0f };（有bug，而且易触发）
					selectedEntityTransform.Scale = scale;

				}

			}
			//! - `ImGuizmo #302`: scale 在物体位于 origin 或相机 perspective 正对物体时会异常，拖动可能导致 transform 全部变 NaN。

		}

		DrawGizmoToolbarOverlay(sceneMin, sceneSize);

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

void EditorLayer::DrawGizmoToolbarOverlay(ImVec2 sceneMin, ImVec2 sceneSize) {

	constexpr float toolbarHeight = 34.0f;		//ToolBar高度
	constexpr float rounding = 5.0f;			//ToolBar矩形圆角
	constexpr float dragHandleWidth = 20.0f;	//拖拽图标的相对偏移
	constexpr float collapsedWidth = 54.0f;		//收纳时的宽度
	constexpr float expandedWidth = 260.0f;		//完全展开时宽度
	constexpr float buttonSize = 22.0f;			//收纳按钮大小

	// 计算实际显示大小
	ImVec2 toolbarSize = {
		m_GizmoToolbarCollapsed ? collapsedWidth : expandedWidth,
		toolbarHeight
	};

	// 基于实际显示大小做出存在范围限制
	float maxOffsetX = std::max(0.0f, sceneSize.x - toolbarSize.x);
	float maxOffsetY = std::max(0.0f, sceneSize.y - toolbarSize.y);
	m_GizmoToolbarOffset.x = std::clamp(m_GizmoToolbarOffset.x, 0.0f, maxOffsetX);
	m_GizmoToolbarOffset.y = std::clamp(m_GizmoToolbarOffset.y, 0.0f, maxOffsetY);

	// ToolBar的屏幕位置
	ImVec2 toolbarPos = {
		sceneMin.x + m_GizmoToolbarOffset.x,
		sceneMin.y + m_GizmoToolbarOffset.y
	};

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddRectFilled(
		toolbarPos,
		ImVec2{ toolbarPos.x + toolbarSize.x, toolbarPos.y + toolbarSize.y },
		IM_COL32(0, 0, 0, 140),
		rounding
	);

	ImGui::SetCursorScreenPos(toolbarPos);
	ImGui::InvisibleButton("##GizmoToolbarDragHandle", ImVec2{ dragHandleWidth, toolbarHeight });	//不可视按钮，但是可以检测到交互意图
	if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {					//判定是否对不可视按钮进行拖拽交互
		ImVec2 delta = ImGui::GetIO().MouseDelta;
		m_GizmoToolbarOffset.x += delta.x;
		m_GizmoToolbarOffset.y += delta.y;
	}

	// 绘制ToolBar拖动手柄（打三行点，每行两个）
	ImVec2 gripCenter = {
		toolbarPos.x + dragHandleWidth * 0.5f,
		toolbarPos.y + toolbarHeight * 0.5f
	};
	for (int row = -1; row <= 1; row++) {
		drawList->AddCircleFilled(ImVec2{ gripCenter.x - 3.0f, gripCenter.y + row * 5.0f }, 1.35f, IM_COL32(210, 210, 210, 220));
		drawList->AddCircleFilled(ImVec2{ gripCenter.x + 3.0f, gripCenter.y + row * 5.0f }, 1.35f, IM_COL32(210, 210, 210, 220));
	}

	// 绘制收纳按钮
	float contentY = toolbarPos.y + (toolbarHeight - buttonSize) * 0.5f;
	ImGui::SetCursorScreenPos(ImVec2{ toolbarPos.x + dragHandleWidth + 5.0f, contentY });
	if (ImGui::Button(m_GizmoToolbarCollapsed ? ">" : "<", ImVec2{ buttonSize, buttonSize })) {
		m_GizmoToolbarCollapsed = !m_GizmoToolbarCollapsed;
	}

	// 平移，选装，缩放，投影选项
	if (!m_GizmoToolbarCollapsed) {
		ImGui::SameLine();
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

		//2D | 3D
		bool is2DMode = m_EditorCamera.GetCameraType() == Ayin::Camera::CameraType::Orthogonal;
		ImVec4 activeTextColor = ImVec4{ 1.0f, 0.75f, 0.25f, 1.0f };	//选中时颜色
		ImVec4 inactiveTextColor = ImVec4{ 0.75f, 0.75f, 0.75f, 1.0f };	//未选中时颜色
		//分割线
		ImGui::SameLine();
		ImGui::TextUnformatted("|");
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Text, is2DMode ? activeTextColor : inactiveTextColor);
		if (ImGui::Button("2D", ImVec2{ 28.0f, buttonSize })) {
			m_EditorCamera.SetEditorCameraMode(Ayin::Camera::CameraType::Orthogonal);
		}
		ImGui::PopStyleColor();

		ImGui::SameLine(0.0f, 2.0f);
		ImGui::TextUnformatted("|");
		ImGui::SameLine(0.0f, 2.0f);

		ImGui::PushStyleColor(ImGuiCol_Text, !is2DMode ? activeTextColor : inactiveTextColor);
		if (ImGui::Button("3D", ImVec2{ 28.0f, buttonSize })) {
			m_EditorCamera.SetEditorCameraMode(Ayin::Camera::CameraType::Perspective);
		}
		ImGui::PopStyleColor();
	}

	// 确保至少可以使用移动手柄
	if (m_GizmoOperation == 0) {
		m_GizmoOperation = ImGuizmo::TRANSLATE;
	}

};


//场景导入导出
void EditorLayer::OpenScene() {

	std::optional<std::string> filePath = Ayin::FileDialogs::OpenFile({ {"Scenen", "json"}}, nullptr);

	if (filePath) {
		m_ActiveScene = Ayin::CreateRef<Ayin::Scene>();


		Ayin::SceneSerializer sceneSerializer{ m_ActiveScene };
		sceneSerializer.Deserializer(*filePath);

		m_ActiveScene->OnViewportResize(m_SceneSize.x, m_SceneSize.y);

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

};

void EditorLayer::NewScene() {

	m_ActiveScene = Ayin::CreateRef<Ayin::Scene>();
	m_SceneHierarchyPanel.SetContext(m_ActiveScene);

	Ayin::Entity mainCamera = m_ActiveScene->CreateEntity("MainCamera");
	mainCamera.GetComponents<Ayin::TransformComponent>().Position.z = 10.0f;
	mainCamera.AddComponent<Ayin::CameraComponent>(Ayin::CameraProp{ .Type{Ayin::Camera::CameraType::Perspective} });

	m_ActiveScene->OnViewportResize(m_SceneSize.x, m_SceneSize.y);

};

void EditorLayer::SaveScene() {

	std::optional<std::string> filePath = Ayin::FileDialogs::SaveFile({ {"Scenen", "json"} }, "Scene");
	
	if (filePath) {
		Ayin::SceneSerializer sceneSerializer{ m_ActiveScene };
		sceneSerializer.Serializer(*filePath);
	} 

};
