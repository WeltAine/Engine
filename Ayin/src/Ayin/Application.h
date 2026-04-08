#pragma once
//应用类
//2026.1.24,修改为单例类


#include "Core.h"
#include "Window.h"
#include "Ayin/Events/ApplicationEvent.h"
#include "Ayin/LayerStack.h"
#include "Ayin/ImGui/ImGuiLayer.h"
#include "Ayin/Renderer/Shader.h"
#include "Ayin/Renderer/Buffer.h"
#include "Ayin/Renderer/VertexArray.h"
#include "Ayin/Renderer/Renderer.h"
#include "Ayin/Renderer/Camera.h"


namespace Ayin {

	//负责更新循环，层管理（层的更新，层的如栈出栈），窗口管理，事件处理（接收，处理和向各层分发）
	class AYIN_API Application
	{

		friend Application* CreatApplication();

	public:
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;


	public:
		inline static Application& Get() { return *s_Instance; };

		inline virtual ~Application() = default;

		void Run();

		void OnEvent(Event& e);
		


		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overLayer);

		inline const Window& GetWindow() { return *m_Window; }

	protected:
		Application();

	private:
		bool OnWindowClose(const WindowCloseEvent& e);
		bool OnWindowResize(const WindowResizeEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_IsVisible = true;					//窗口内容是否可见（好像没什么必要的样子，或者说这个字段应该移动到Window本身中）
		bool m_Running = true;

		LayerStack m_LayerStack;					//栈层
		ImGuiLayer* m_ImGuiLayer;//！！！没有释放

		float m_LastFrameTime = 0.0f;

		
	
	private:
		static Application* s_Instance;				//应用单例实例

	};


	/// <summary>
	/// 实际上这成为了桥接模式（Application为抽象，SandBox为实现）中给客户端（在这里是main方法，位于EntryPoint.h中）的接口字段配置具体实例的一种方式，与Windos.h中的Creat()类似
	/// </summary>
	/// <returns>返回抽象Application指针</returns>
	extern Application* CreatApplication();//由Ayin应用实现（即客户端侧）
	//这样声明，但用extern来指示外部寻找，就导致引入该头文件的客户端侧必须提供实现，否则报错
	//它完成了桥接模式中向客户端的抽象字段完成具体实例的配置过程

	//inline Application* Application::s_Instance = nullptr;
}


