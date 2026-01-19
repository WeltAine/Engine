#include "AyinPch.h"

#include "Application.h"

#include "spdlog/fmt/bundled/format.h"

#include "Ayin/Events/ApplicationEvent.h"
#include "Ayin/Log.h"

#include <GLFW/glfw3.h>

namespace Ayin {


	Application::Application() 
	{
		m_Window = std::unique_ptr<Window>(Window::Create());

		//当窗口发生事件时（通过GLFW），窗口类会调用OnEvent回调，由我们来处理，Application成了中介者，而窗口成为了组件，这道函指则是组件与中介之间的沟通桥梁
		m_Window->SetEventCallback(BIND_EVENT_FUN(Application::OnEvent, this, std::placeholders:;_1));//通过
	};

	Application::~Application() {};

	void Application::Run() {

		//测试事件
		WindowResizeEvent e(1280, 720);
		if (e.IsInCatagory(EventCategory::EventCategoryApplication)) {
			AYIN_INFO(e);
		}
		if (e.IsInCatagory(EventCategory::EventCategoryInput)) {
			AYIN_INFO(e);
		}

		while (m_Running)
		{
			//测试窗口
			glClearColor(1, 0, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e) {

		//输出日志（除关闭以外的事件先这么处理）
		AYIN_CORE_TRACE("{0}", e);

		EventDispatcher dispatcher(e);

		//关闭事件
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FUN(Application::OnWindowClose));

	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {

		m_Running = false;
		return true;
	}

}