#include "AyinPch.h"
#include "ImGuiLayer.h"
#include "Ayin/Application.h"

#include <GLFW/glfw3.h>

#include "imgui.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glad/glad.h>

//#include "Platform/OpenGL/imgui_impl_opengl3.h"
//#include "Ayin/ImGui/imgui_impl_glfw.h"

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
		//???
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard
			| ImGuiConfigFlags_NavEnableGamepad
			| ImGuiConfigFlags_DockingEnable
			| ImGuiConfigFlags_ViewportsEnable
			//| ImGuiConfigFlags_ViewportsNoTaskBarIcons
			//| ImGuiConfigFlags_ViewportsNoMerge
			;
		//cherno旧版本的最后两个ConfigFlags枚举在新版本中的这个位置
		io.ConfigViewportsNoAutoMerge = true;               // 替代原ImGuiConfigFlags_ViewportsNoMerge
		io.ConfigViewportsNoTaskBarIcon = true;				// 替换原ImGuiConfigFlags_ViewportsNoTaskBarIcons
		
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {//???
			ImGuiStyle& style = ImGui::GetStyle();
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		GLFWwindow* windos = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		ImGui_ImplGlfw_InitForOpenGL(windos, true);//该方法主要配置ImGui中的后端上下文，简单来说就是接入（取得）GLFW的数据（大概，哈哈哈）
		////设定特定窗口系统（这里为 GLFW）具备哪些功能
		//io.BackendFlags								
		//	|= ImGuiBackendFlags_HasMouseCursors//鼠标光标
		//	| ImGuiBackendFlags_HasSetMousePos;//设置鼠标位置
		//由ImGui_ImplGlfw_InitForOpenGL

		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiLayer::OnDetach() {
		//???
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}


	void ImGuiLayer::OnImGuiRender() {
		ImGuiIO& io = ImGui::GetIO();

		//我记得是交由某个NewFrame中完成吧
		//Application& app = Application::Get();
		//io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());
		//float time = (float)glfwGetTime();
		//io.DeltaTime = m_Time > 0.0f ? (time - m_Time) : (1.0f / 60.0f);
		//m_Time = time;


		static bool show = true;
		if (show)
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

			glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		}

		//ImGui::Render();
		//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


	}

	void ImGuiLayer::Begin() {
		//???
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End() {
		//???
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetWidth());

		//渲染
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}


}