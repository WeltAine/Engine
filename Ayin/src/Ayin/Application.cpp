#include "AyinPch.h"

#include "Application.h"

#include "Ayin/Events/ApplicationEvent.h"
#include "Ayin/Log.h"

namespace Ayin {


	Application::Application() {};

	Application::~Application() {};

	void Application::Run() {


		WindowResizeEvent e(1280, 720);
		if (e.IsInCatagory(EventCategory::EventCategoryApplication)) {
			AYIN_INFO(e);
		}
		if (e.IsInCatagory(EventCategory::EventCategoryInput)) {
			AYIN_INFO(e);
		}

		while (true)
		{

		}
	}

}