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
		
		//io.KeysData[ImGuiKey_Tab] = GLFW_KEY_TAB;
		//新的ImGui中已经为我们设置好了映射ImGui_ImplGlfw_KeyCallback与ImGui_ImplGlfw_KeyToImGuiKey方法
		//实际上新旧ImGui键盘映射之间有很大理念上的不同，旧版中ImGui其实并没有键盘布局意识，它的io.KeysDown中的每一个元素对应的么字母完全由GLFW中的GLFW_KEY来决定，GLFW在这个位置是什么按键就是什么按键
		//所以在旧版ImGui中读取按键状态时使用的仍旧是GLFW_KEY作为自己的索引，Cherno设置的io.KeysMap是ImGui到GLFW_KEY的映射，是让ImGui在使用时知道某个ImGuiKey要到那个索引中去找。这个索引由GLFW_KEY来决定，寄人篱下的既视感
		//而新版的ImGui有了自己的布局意识，io.KeysData中的元素都稳定对应一个按键。这样的平台无关性，就需要一个中间的实践层来做转换ImGui_ImplGlfw_KeyToImGuiKey方法就是这么用的


		//GLFWwindow* window = glfwGetCurrentContext();
		//ImGui_ImplGlfw_InitForOpenGL(window, true);//创建ImGui平台后端
		////该方法的第二个参数是是否让ImGui的后端是否接入事件系统（GLFW是C库，它只能放入一个回调，业界通解是，构成一个回调链）
		////这里使用true就是表明我们已经加入过自己的回调，通过回调链的方式将ImGui也接入GLFW的事件系统中，如果为false则不会接入。（详情可阅读后端实现源码）
		////我们也可以滞后接入，使用IMGUI_IMPL_API void     ImGui_ImplGlfw_InstallCallbacks(GLFWwindow* window);这样时机就相当随随意了。
		////总之这么干可以一定程度上让ImGui_Impl_glfw成为我们的ImGuiLayer，因为事件处理被它实现，但这样会打乱Cherno的事件产生和执行顺序，PollEvent->App::OnEvent()->各层的处理
		////不过我们还是尽可能贴近Cherno，这段注释会在本次提交后被删除

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
#pragma region 非ImGuiIO直接写入
		ImGuiIO& io = ImGui::GetIO();
		ImGuiKey key = ImGui_ImplGlfw_KeyToImGuiKey(event.GetKeyCode(), event.GetScanCode());

		io.AddKeyEvent(key, true);
#pragma endregion

#pragma region IO直接写入
		//ImGuiIO& io = ImGui::GetIO();
		//ImGuiKey key = ImGui_ImplGlfw_KeyToImGuiKey(event.GetKeyCode(), event.GetScanCode());
		//io.KeysData[key].Down = true;

		//io.KeyCtrl = io.KeysData[ImGuiKey_LeftCtrl].Down || io.KeysData[ImGuiKey_RightCtrl].Down;
		//io.KeyShift = io.KeysData[ImGuiKey_LeftShift].Down || io.KeysData[ImGuiKey_RightCtrl].Down;
		//io.KeyAlt = io.KeysData[ImGuiKey_LeftAlt].Down || io.KeysData[ImGuiKey_RightAlt].Down;
		//io.KeySuper = io.KeysData[ImGuiKey_LeftSuper].Down || io.KeysData[ImGuiKey_RightSuper].Down;
#pragma endregion

		return false;
	}

	bool ImGuiLayer::OnKeyReleasedEvent(KeyReleasedEvent& event)
	{
#pragma region 非ImGuiIO直接写入
		ImGuiIO& io = ImGui::GetIO();
		ImGuiKey key = ImGui_ImplGlfw_KeyToImGuiKey(event.GetKeyCode(), event.GetScanCode());

		io.AddKeyEvent(key, false);
#pragma endregion

#pragma region IO直接写入
		//ImGuiIO& io = ImGui::GetIO();
		//io.KeysData[event.GetKeyCode()].Down = false;
#pragma endregion

		return false;
	}

	bool ImGuiLayer::OnTextEvent(TextEvent& event) {

		ImGuiIO& io = ImGui::GetIO();

		io.AddInputCharacter(event.GetKeyCode());

		return false;
	}

	bool ImGuiLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
	{
#pragma region 非ImGuiIO直接写入
		ImGuiIO& io = ImGui::GetIO();

		GLFWwindow* window = glfwGetCurrentContext();
		io.AddKeyEvent(ImGuiMod_Ctrl, (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS));
		io.AddKeyEvent(ImGuiMod_Shift, (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS));
		io.AddKeyEvent(ImGuiMod_Alt, (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS));
		io.AddKeyEvent(ImGuiMod_Super, (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS));

		io.AddMouseButtonEvent(event.GetMouseButton(), true);
#pragma endregion

#pragma region IO直接写入
		//ImGuiIO& io = ImGui::GetIO();
		//io.MouseDown[event.GetMouseButton()] = true;
#pragma endregion

		return false;
	}

	bool ImGuiLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& event)
	{
#pragma region 非ImGuiIO直接写入
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseButtonEvent(event.GetMouseButton(), false);
#pragma endregion

#pragma region IO直接写入
		//ImGuiIO& io = ImGui::GetIO();
		//io.MouseDown[event.GetMouseButton()] = false;
		//io.MouseSource = e->MouseButton.MouseSource;
#pragma endregion

		return false;
	}

	bool ImGuiLayer::OnMouseMovedEvent(MouseMovedEvent& event)
	{
#pragma region 非ImGuiIO直接写入
		ImGuiIO& io = ImGui::GetIO();
		io.AddMousePosEvent(event.GetMouseX(), event.GetMouseY());
#pragma endregion

#pragma region IO直接写入
		//ImGuiIO& io = ImGui::GetIO();
		//io.MousePos = ImVec2(event.GetMouseX(), event.GetMouseY());
		//io.MouseSource = e->MousePos.MouseSource;
#pragma endregion

		return false;
	}

	bool ImGuiLayer::OnMouseSrolled(MouseSrolledEvent& event)
	{
#pragma region 非ImGuiIO直接写入
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseWheelEvent(event.GetXoffset(), event.GetYoffset());
#pragma endregion

#pragma region IO直接写入
		//ImGuiIO& io = ImGui::GetIO();
		//io.MouseWheelH += event.GetXoffset();
		//io.MouseWheel += event.GetYoffset();
		//io.MouseSource = e->MousePos.MouseSource;
#pragma endregion

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