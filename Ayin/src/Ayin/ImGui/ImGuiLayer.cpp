#include "AyinPch.h"
#include "ImGuiLayer.h"
#include "Ayin/Application.h"

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "Platform/OpenGL/imgui_impl_opengl3.h"

namespace Ayin {

	ImGuiLayer::ImGuiLayer() 
		:Layer("ImGuiLayer")
	{}


	ImGuiLayer::~ImGuiLayer() {}


	void ImGuiLayer::OnAttach()
	{
		//我对Imgui的感受就是一个UI框架，平台后端和渲染后端组成，它是一套基于实际平台和渲染的图形框架
		//就是我们现在在做的引擎一样，但是它只负责UI，利用具体的事件系统，利用具体的图形API
		

		ImGui::CreateContext();//创建ImGuiContext
		ImGui::StyleColorsDark();//设置风格

		ImGuiIO& io = ImGui::GetIO();//通过io向ImGui提供数据

		//设定特定窗口系统（这里为 GLFW）具备哪些功能
		io.BackendFlags								
			|= ImGuiBackendFlags_HasMouseCursors//鼠标光标
			| ImGuiBackendFlags_HasSetMousePos;//设置鼠标位置
		
		//io.KeysData[ImGuiKey_Tab] = GLFW_KEY_TAB;
		//新的ImGui中已经为我们设置好了映射ImGui_ImplGlfw_KeyCallback与ImGui_ImplGlfw_KeyToImGuiKey方法
		//所以我们应该是不用配的？？？文件要不要引入


		ImGui_ImplOpenGL3_Init("#version 410");

	}

	void ImGuiLayer::OnDetach() {}

	void ImGuiLayer::OnUpdate()
	{
		ImGuiIO& io = ImGui::GetIO();

		Application& app = Application::Get();

		io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

		float time = (float)glfwGetTime();

		io.DeltaTime = m_Time > 0.0f ? (time - m_Time) : (1.0f / 60.0f);
		m_Time = time;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		static bool show = true;
		ImGui::ShowDemoWindow();


		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


	}

	void ImGuiLayer::OnEvent(Event& event) {}

}