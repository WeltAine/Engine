#include "AyinPch.h"
#include "ImGuiLayer.h"
#include "Ayin/Application.h"

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "Platform/OpenGL/imgui_impl_opengl3.h"

#include "Ayin/ImGui/imgui_impl_glfw.h"
#include <glad/glad.h>


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


		GLFWwindow* window = glfwGetCurrentContext();

		//ImGui_ImplGlfw_InitForOpenGL(window, true);//创建ImGui平台后端
		////该方法的第二个参数是是否让ImGui的后端是否接入事件系统（GLFW是C库，它只能放入一个回调，业界通解是，构成一个回调链）
		////这里使用true就是表明我们已经加入过自己的回调，通过回调链的方式将ImGui也接入GLFW的事件系统中，如果为false则不会接入。（详情可阅读后端实现源码）
		////我们也可以滞后接入，使用IMGUI_IMPL_API void     ImGui_ImplGlfw_InstallCallbacks(GLFWwindow* window);这样时机就相当随随意了。
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
		ImGui_ImplGlfw_NewFrame();//？？？
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


}