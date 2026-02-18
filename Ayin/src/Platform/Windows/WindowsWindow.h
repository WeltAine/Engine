#pragma once

#include "Ayin/Window.h"

#include "Ayin/Renderer/GraphicsContext.h"
#include <GLFW/glfw3.h>

namespace Ayin {

	class WindowsWindow : public Window//也没加AYIN_API，会不会是那个属性也有继承性？？？
	{
		friend class Window;
	public:
		// 通过 Window 继承
		virtual void OnUpdate() override;

		virtual void* GetNativeWindow() const { return m_Window; };

		virtual unsigned int GetWidth() const override { return m_Data.Width; };
		virtual unsigned int GetHeight() const override { return m_Data.Height; };

		//设置观察者（回调），该观察者用于Event分发
		virtual void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; };

		//垂直同步
		virtual void SetVSync(bool enabled) override;
		virtual bool IsVSync() const override;

		virtual ~WindowsWindow();

	protected:
		WindowsWindow(const WindowProps& props);

	private:
		//为什么不写在上一层呢
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();


	private:
		GLFWwindow* m_Window;
		GraphicsContext* m_GraphicsContext;


		//交给GLFW的数据（以void*形式），主要用在系统事件的回调中，类似于闭包
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


