#include "AyinPch.h"
#include "ImGuiLayer.h"
#include "Ayin/Application.h"

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "Platform/OpenGL/imgui_impl_opengl3.h"

//#include "Ayin/ImGui/imgui_impl_glfw.h"
#include <glad/glad.h>

#pragma region 追加ImGui_GLFW平台后端方法

ImGuiKey ImGui_ImplGlfw_KeyToImGuiKey(int keycode, int scancode);//引入声明

#pragma endregion

namespace Ayin {

	ImGuiLayer::ImGuiLayer() 
		:Layer("ImGuiLayer")
	{}


	ImGuiLayer::~ImGuiLayer() {}


	void ImGuiLayer::OnAttach()
	{
		//我对Imgui的感受就是一个UI框架，平台后端和渲染后端组成，它是一套基于实际平台和渲染的图形框架
		//就是我们现在在做的引擎一样，但是它只负责自己的UI，利用具体的事件系统，利用具体的图形API
		

		ImGui::CreateContext();//创建ImGuiContext
		ImGui::StyleColorsDark();//设置风格

		ImGuiIO& io = ImGui::GetIO();//通过io向ImGui提供数据

		//设定特定窗口系统（这里为 GLFW）具备哪些功能
		io.BackendFlags								
			|= ImGuiBackendFlags_HasMouseCursors//鼠标光标
			| ImGuiBackendFlags_HasSetMousePos;//设置鼠标位置
		

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

		ImGui_ImplOpenGL3_NewFrame();//？？？
		//ImGui_ImplGlfw_NewFrame();//？？？
		ImGui::NewFrame();//？？？

		static bool show = true;
		if(show)
			ImGui::ShowDemoWindow(&show);


		// Our state
		static bool show_another_window = false;
		static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);//颜色
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("文本");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();


			if (show_another_window)
			{
				ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
				ImGui::Text("Hello from another window!");
				if (ImGui::Button("Close Me"))
					show_another_window = false;
				ImGui::End();
			}

		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);

	}


	void ImGuiLayer::OnEvent(Event& event) {

		//构造事件调度器
		EventDispatcher dispatcher(event);

		//将事件发送给本层的每一个事件处理方法
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FUN(ImGuiLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FUN(ImGuiLayer::OnKeyReleasedEvent));
		dispatcher.Dispatch<TextEvent>(BIND_EVENT_FUN(ImGuiLayer::OnTextEvent));

		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FUN(ImGuiLayer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FUN(ImGuiLayer::OnMouseButtonReleasedEvent));
		dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FUN(ImGuiLayer::OnMouseMovedEvent));

		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FUN(ImGuiLayer::OnWindowResizeEvent));
	}

	bool ImGuiLayer::OnKeyPressedEvent(KeyPressedEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGuiKey key = ImGui_ImplGlfw_KeyToImGuiKey(event.GetKeyCode(), event.GetScanCode());

		io.AddKeyEvent(key, true);

		return false;
	}

	bool ImGuiLayer::OnKeyReleasedEvent(KeyReleasedEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGuiKey key = ImGui_ImplGlfw_KeyToImGuiKey(event.GetKeyCode(), event.GetScanCode());

		io.AddKeyEvent(key, false);

		return false;
	}

	bool ImGuiLayer::OnTextEvent(TextEvent& event) {

		ImGuiIO& io = ImGui::GetIO();

		io.AddInputCharacter(event.GetKeyCode());

		return false;
	}

	bool ImGuiLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();

		GLFWwindow* window = glfwGetCurrentContext();
		io.AddKeyEvent(ImGuiMod_Ctrl, (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS));
		io.AddKeyEvent(ImGuiMod_Shift, (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS));
		io.AddKeyEvent(ImGuiMod_Alt, (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS));
		io.AddKeyEvent(ImGuiMod_Super, (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS));

		io.AddMouseButtonEvent(event.GetMouseButton(), true);

		return false;
	}

	bool ImGuiLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseButtonEvent(event.GetMouseButton(), false);

		return false;
	}

	bool ImGuiLayer::OnMouseMovedEvent(MouseMovedEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMousePosEvent(event.GetMouseX(), event.GetMouseY());

		return false;
	}

	bool ImGuiLayer::OnMouseSrolled(MouseSrolledEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseWheelEvent(event.GetXoffset(), event.GetYoffset());

		return false;
	}

	bool ImGuiLayer::OnWindowResizeEvent(WindowResizeEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
		glViewport(0, 0, event.GetWidth(), event.GetHeight());

		return false;
	}


}