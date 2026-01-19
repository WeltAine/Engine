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

}