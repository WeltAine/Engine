#include "AyinPch.h"
#include "WindowsWindow.h"

 
namespace Ayin {

	static bool s_GLFWInitialized = false;//专制变量，确保glfw只初始化一次

	Window* Window::Create(const WindowProps& props) {
		return  new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props) 
		//:m_Data{props.Title, props.Width, props.Height}
	{
		Init(props);
	}

	void WindowsWindow::Init(const WindowProps& props) {

		//存储窗口数据
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;


		//GLWL初始化
		if (!s_GLFWInitialized) {

			int success = glfwInit();
			AYIN_CORE_ASSERT(success, "Could not initialize GLFW!");//断言宏

			s_GLFWInitialized = true;
		}

		//创建窗口
		//GLFW相关的需要找时间补补
		m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);//猜测：创建窗口
		glfwMakeContextCurrent(m_Window);//猜测：构建交互上下文
		glfwSetWindowUserPointer(m_Window, &m_Data);//猜测：上下文映射

		//其余窗口属性设置
		SetVSync(true);
	}


	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();//猜测：？？？
		glfwSwapBuffers(m_Window);//猜测：交换帧缓冲（可能是双缓冲方案）
	}

	void WindowsWindow::Shutdown() {
		glfwDestroyWindow(m_Window);
	}

	WindowsWindow::~WindowsWindow() {
		Shutdown();
	}


	//unsigned int WindowsWindow::GetWidth() const { return m_Data.Width; }

	//unsigned int WindowsWindow::GetHeight() const { return m_Data.Height; }



	//void WindowsWindow::SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }



	void WindowsWindow::SetVSync(bool enabled) 
	{ 
		if (enabled) {
			glfwSwapInterval(1);
		}
		else {
			glfwSwapInterval(0);
		}

		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const { return m_Data.VSync; }

}
