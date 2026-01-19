#pragma once

#include "Ayin/Window.h"

#include <GLFW/glfw3.h>

namespace Ayin {

	class WindowsWindow : public Window//也没加AYIN_API，会不会是那个属性也有继承性？？？
	{
		friend class Window;
	public:
		// 通过 Window 继承
		void OnUpdate() override;

		unsigned int GetWidth() const override { return m_Data.Width; };
		unsigned int GetHeight() const override { return m_Data.Height; };

		//设置观察者（回调），该观察者用于Event分发
		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; };

		//垂直同步
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		virtual ~WindowsWindow();

	protected:
		WindowsWindow(const WindowProps& props);

	private:
		//为什么不写在上一层呢
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();


	private:
		GLFWwindow* m_Window;


		//？？？猜测——窗口与GLFW之间的上下文文本类型
		struct WindowData {
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			//当事件发生时，所要触发的回调，交由该回调处理，窗口此时成了组件，而观察者成了组件与中介者之间的一种连接方式
			EventCallbackFn EventCallback;//与Application的函数指针连接方式
		};

		WindowData m_Data;
	};
}


