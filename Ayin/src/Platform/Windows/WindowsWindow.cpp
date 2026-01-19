#include "AyinPch.h"
#include "WindowsWindow.h"

#include "Ayin/Events/KeyEvent.h"
#include "Ayin/Events/MouseEvent.h"
#include "Ayin/Events/ApplicationEvent.h"

 
namespace Ayin {

	static bool s_GLFWInitialized = false;//专制变量，确保glfw只初始化一次

	static void GLFWErrorCallback(int error_code, const char* description) {//不写成无闭包lambda或许是为了复用
		AYIN_CORE_ERROR("GLFW Error ({0}) : {1}", error_code, description);
	}



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
			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized = true;
		}

		//创建窗口
		//GLFW相关的需要找时间补补
		m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);//猜测：创建窗口
		glfwMakeContextCurrent(m_Window);//猜测：构建交互上下文
		glfwSetWindowUserPointer(m_Window, &m_Data);//猜测：上下文映射？？？在写GLFW到引擎侧事件系统的中间层时我有了一点猜想，GLFW是C库一些特性是不支持的，比如lambda的闭包（这在C#中是默认的，让Lambda可以表现出“函数穿透，以访问对象数据”的效果，但是lambda一旦带上了闭包那就无法转换成C风格的函数指针，所以使用一个数据块代替，用以存储外侧的东西。闭包的本质或许就是一个外侧数据块，以此来打到穿透现象）

		//其余窗口属性设置
		SetVSync(true);



		//连接GLFW的回调与我们事件系统，以达成产生GLFW窗口在交互时产生事件的效果，这里是GLFW-》引擎侧事件系统的中间层
		//发送键盘事件
		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)//按键，不认识，动作，修饰（应该是类似ctrl这类的）
			{
				switch (action) {
					case GLFW_PRESS: {
						WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);//可以看一看glfwSetWindowUserPointer(m_Window, &m_Data)处的猜想

						KeyPressedEvent event(key, 0);
						data.EventCallback(event);
						break;
					}
					case GLFW_REPEAT: {
						WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);//可以看一看glfwSetWindowUserPointer(m_Window, &m_Data)处的猜想

						KeyPressedEvent event(key, 1);
						data.EventCallback(event);
						break;
					}
					case GLFW_RELEASE: {
						WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);//可以看一看glfwSetWindowUserPointer(m_Window, &m_Data)处的猜想

						KeyReleasedEvent event(key);
						data.EventCallback(event);
						break;
					}
				}
			});


		//发送鼠标事件
		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);//可以看一看glfwSetWindowUserPointer(m_Window, &m_Data)处的猜想

				MouseMovedEvent event((float)xpos, (float)ypos);
				data.EventCallback(event);
			});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);//可以看一看glfwSetWindowUserPointer(m_Window, &m_Data)处的猜想

				switch (action) {
					case GLFW_PRESS: {
						WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);//可以看一看glfwSetWindowUserPointer(m_Window, &m_Data)处的猜想

						MouseButtonPressedEvent event(button);
						data.EventCallback(event);
						break;
					}
					case GLFW_RELEASE: {
						WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);//可以看一看glfwSetWindowUserPointer(m_Window, &m_Data)处的猜想

						MouseButtonReleasedEvent event(button);
						data.EventCallback(event);
						break;
					}
				}

			});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset)//？？？滚轮，是那个滚珠么？
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);//可以看一看glfwSetWindowUserPointer(m_Window, &m_Data)处的猜想

				MouseSrolledEvent event((float)xoffset, (float)yoffset);
				data.EventCallback(event);
			});


		//发送窗口事件
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);//可以看一看glfwSetWindowUserPointer(m_Window, &m_Data)处的猜想

				WindowResizeEvent event(width, height);
				data.EventCallback(event);
			});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);//可以看一看glfwSetWindowUserPointer(m_Window, &m_Data)处的猜想

				WindowCloseEvent event;
				data.EventCallback(event);
			});
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
