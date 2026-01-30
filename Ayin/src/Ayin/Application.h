#pragma once
//应用类
//2026.1.24,修改为单例类


#include "Core.h"
#include "Window.h"
#include "Ayin/Events/ApplicationEvent.h"
#include "Ayin/LayerStack.h"

namespace Ayin {

	
	class AYIN_API Application
	{

		friend Application* CreatApplication();

	public:
		inline static Application& Get() { return *s_Instance; };

		virtual ~Application();

		void Run();

		void OnEvent(Event& e);
		
		bool OnWindowClose(WindowCloseEvent& e);


		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overLayer);

		inline Window& GetWindow() { return *m_Window; }

	protected:
		Application();

	private:
		static Application* s_Instance;//应用单例实例

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;

		LayerStack m_LayerStack;//栈层
	};


	Application* CreatApplication();//由Ayin应用实现（即客户端侧）
	//这样声明，但用extern来指示外部寻找，就导致引入该头文件的客户端侧必须提供实现，否则报错
	//所以这相当于一个步骤模式中的步骤，不过不那么典型就是了，也可以完全不这么想，这样的看法确实有些多余

	//inline Application* Application::s_Instance = nullptr;
}


