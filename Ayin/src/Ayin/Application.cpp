#include "AyinPch.h"

#include "Application.h"

#include "spdlog/fmt/bundled/format.h"

#include "Ayin/Events/ApplicationEvent.h"
#include "Ayin/Log.h"

#include "Ayin/Input.h"

#include "Ayin/Renderer/Buffer.h"

#include "Ayin/Core/Timestep.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Ayin {

	Application* Application::s_Instance = nullptr;



	/// <summary>
	/// 创建窗口，单例，层栈以及为窗口的人造闭包配置事件函指
	/// </summary>
	Application::Application()
	{
		AYIN_ASSERT(!s_Instance, "Application already exists!");//断言，防止破坏单例
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());

		//当窗口发生事件时（通过GLFW），窗口类会调用OnEvent回调，由我们来处理，Application成了中介者，而窗口成为了组件，这道函指则是组件与中介之间的沟通桥梁
		m_Window->SetEventCallback(BIND_EVENT_FUN(Application::OnEvent));

		//栈层使用默认构造
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);//暂时没有空间清理的过程，可能泄露（在拆出层时）,感觉该层的所有权有写模糊啊

	};


	void Application::Run() {

		while (m_Running)
		{
			// 时间更新
			Timestep runTime = glfwGetTime();//已经运行的时间长度
			Timestep frameInterval = runTime - m_LastFrameTime;
			//? 这个过程我觉得应该单独设置一个Time外观来完成，甚至是单独弄一个为其弄一个计时层和ImGui一样，一起更新
			//? glfwGetTime这类底层API应该封在一个Platform中，像Input和WindowsInput那样
			AYIN_CORE_INFO("FrameInterval:{0}s  ({1}ms)", float(frameInterval), frameInterval.GetMilliseconds());


			// 层更新
			for (Layer* layer : m_LayerStack) {
				layer->OnUpdate(frameInterval);
			}

			// 渲染ImGui（之后会单独放到渲染线程上，所以不会在Layer的OnUpdate中执行）
			// m_ImGuiLauer的Begin和End中为ImGui上下文的相关设置，详情可查看函数
			// 这里应该接收的是上一帧的情况（因为事件本帧事件会在Window的OnUpdate中触发）
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack) {
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();
			
			// 窗口更新
			m_Window->OnUpdate();

			m_LastFrameTime = runTime;

		}
	}

	void Application::OnEvent(Event& e) {

		//输出日志（除关闭以外的事件先这么处理）
		AYIN_CORE_TRACE("{0}", e);

		EventDispatcher dispatcher(e);

		//关闭事件
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FUN(Application::OnWindowClose));


		//从后往前逐层通知事件
		for (auto iterator = m_LayerStack.end(); iterator != m_LayerStack.begin(); ) {

			(*--iterator)->OnEvent(e);
			if (e.handled) {
				break;
			}
		}

	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {

		m_Running = false;
		return true;
	}


	void Application::PushLayer(Layer* layer) {
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overLayer) {
		m_LayerStack.PushOverlay(overLayer);
	}

}