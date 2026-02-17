#include "AyinPch.h"
#include "ImGuiLayer.h"
#include "Ayin/Application.h"

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

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
		//ConfigFlags是ImGui的功能设置选项（想怎么用ImGui），而BackendFlags则是告知ImGui实际的后端支持什么能力
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard
			| ImGuiConfigFlags_NavEnableGamepad
			| ImGuiConfigFlags_DockingEnable
			| ImGuiConfigFlags_ViewportsEnable
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
		// 负责编译 ImGui 用于自身渲染的着色器。告知 GPU 如何绘制构成窗口和按钮的三角形。
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiLayer::OnDetach() {
		
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();//简单来说清理ImGui在初始化时创建的GLFW资源，关闭回调，关闭光标，接触window与UIContext的绑定
		ImGui::DestroyContext();
	}


	void ImGuiLayer::OnImGuiRender() {
		ImGuiIO& io = ImGui::GetIO();

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

	}

	void ImGuiLayer::Begin() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		// 总的来说更新UIContext；获取窗口（大小，屏幕缩放）数据，更新（检查）显示器设置，更新deltatime，更新鼠标状态，光标状态，更新（检查）手柄设置

		ImGui::NewFrame();//以下内容可能存在谬误，请谅解
		// 部分全局状态清理（按键状态记录，绘制指令缓存）
		// 事件处理（后端实现的GLFW链式回调中ImGui是走了一个AddXXXEvent，将事件加入一个队列，会在NewFrame中进行一次性处理，对UIContext的状态做一些设置）
		// 根据GLFW_NewFrame()中记录的数据和事件的预处理结果设置UIContext（计算帧率；按键数据；是否响应（聚焦））
		// 清空上一帧的元素布局信息（所以这里应该是有第一阶段的Hits Test的）
	}

	void ImGuiLayer::End() {

		ImGuiIO& io = ImGui::GetIO();

		//渲染
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			// 更新所有 ImGui 独立窗口的位置、大小（比如拖动一个 ImGui 窗口到副屏，这一步会同步位置）。
			ImGui::RenderPlatformWindowsDefault();
			// 把这些独立的 ImGui 窗口也渲染出来（如果不开多视口，这一步不需要）。
			glfwMakeContextCurrent(backup_current_context);
		}
	}


}