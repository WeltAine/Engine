#include "AyinPch.h"

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include "WindowsWindow.h"

#include "Ayin/Events/KeyEvent.h"
#include "Ayin/Events/MouseEvent.h"
#include "Ayin/Events/ApplicationEvent.h"


//#include "Ayin/ImGui/imgui_impl_glfw.h"

namespace Ayin {

	static bool s_GLFWInitialized = false;//专制变量，确保glfw只初始化一次

	static void GLFWErrorCallback(int error_code, const char* description) {//不写成无闭包lambda或许是为了复用
		AYIN_CORE_ERROR("GLFW Error ({0}) : {1}", error_code, description);
	}



	Window* Window::Create(const WindowProps& props) {
		return  new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props) 
	{
		Init(props);
	}

	void WindowsWindow::Init(const WindowProps& props) {

		//存储窗口数据
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;


		// GLWL初始化
		#pragma region GLFW初始化与相关配置（OpenGL版本以及错误回调）
		if (!s_GLFWInitialized) {
			glfwSetErrorCallback(GLFWErrorCallback);

			int success = glfwInit();
			// 在使用大多数 GLFW 函数之前，必须先对库进行初始化。如果初始化成功，将返回 GLFW_TRUE ；如果发生错误，则返回 GLFW_FALSE 。
			// 初始化过程会检查当前设备的可用特性、枚举显示器、初始化计时器，并执行所有必要的平台相关初始化操作。
			// 在使用完 GLFW 后，通常在应用程序退出前，你需要调用终止函数glfwTerminate()来释放 GLFW 资源。

			AYIN_CORE_ASSERT(success, "Could not initialize GLFW!");//断言宏
			s_GLFWInitialized = true;

			// OpenGL上下文版本，与配置文件
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		}
		#pragma endregion

		// 创建窗口
		m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
		// 只需调用一次 glfwCreateWindow 函数即可创建窗口及其 OpenGL 上下文，该函数会返回一个指向窗口与上下文组合（对没错，是组合模式）对象的句柄
		// 当不再需要窗口和上下文时，请使用glfwDestroyWindow(window)将其销毁。
		// GLFW在创建窗口时也会创建OpenGL/Vulkan上下文（这是让GPU运作的关键之一，类似我们在shader编程中提及的渲染状态信息）
		// 另一件重要的事情是，操作系统只有在创建了窗口（准确的来说是窗口的设备上下文）后才会给与访问显卡驱动的可能（这些驱动由硬件厂商编写，遵循特定规范，比如OpenGL就是一种规范）
		// 而我们的Glad库核心是加载器和一堆函数指针（加载器用于将驱动中的方法记录到函数指针中，而查找驱动方法的手段有GLFW提供，可能是因为窗口是它创建的，它知道如何访问驱动）

		// 将 OpenGL 上下文设置为当前上下文
		// 在使用 OpenGL API 之前，必须先创建一个当前的 OpenGL 上下文（Context）。该上下文将一直保持当前状态，直到你切换到另一个上下文，或者该上下文所属的窗口被销毁。
		glfwMakeContextCurrent(m_Window);//将当前线程与窗口绑定，该线程所执行的gl执行都将由该窗口的OpenGL/Vulkan上下文处理。

		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);//正如我们前面所说，glad的核心功能之一是记录所有的现代OpenGL驱动方法，以便我们使用（否则我们就需要自己用GLFW的辅助方法拉驱动，用驱动，应该也是有的，可能比较老？），但是查找驱动的方法由GLFW提供，因为窗口是GLFW创建的，它知道是什么操作系统，该如何访问驱动
		// 加载器需要绑定当前上下文（Current Context）才能工作。

		AYIN_CORE_ASSERT(status, "Failed to initialize GLAFD");

		glfwSetWindowUserPointer(m_Window, &m_Data);
		// 猜测：上下文映射？？？
		// 在写GLFW到引擎侧事件系统的中间层时我有了一点猜想，GLFW是C库一些特性是不支持的
		// 比如lambda的闭包（这在C#中是默认的，让Lambda可以表现出“函数穿透，以访问对象数据”的效果，但是lambda一旦带上了闭包那就无法转换成C风格的函数指针，所以使用一个数据块代替，用以存储外侧的东西。闭包的本质或许就是一个外侧数据块，以此来打到穿透现象）
		// 所以我们写的WindowData就类似一种手动创建的“闭包”


		// 其余窗口属性设置
		SetVSync(true);


		// 连接GLFW的回调与我们事件系统，以达成产生GLFW窗口在交互时产生事件的效果，这里是GLFW-》引擎侧事件系统的中间层
		// 无论是按键按下、GLFW 窗口移动还是错误发生，大多数事件都是通过回调函数来报告的。
		// 回调函数是 C 函数（或 C++ 静态方法），由 GLFW 在触发事件时调用，并传入描述该事件的相关参数。
		
		#pragma region GLFW事件回调设置
		//发送键盘事件
		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)//按键，不认识，动作，修饰（应该是类似ctrl这类的）
			{
				switch (action) {
					case GLFW_PRESS: {
						WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);//可以看一看glfwSetWindowUserPointer(m_Window, &m_Data)处的猜想

						KeyPressedEvent event(key, scancode, 0);
						data.EventCallback(event);
						break;
					}
					case GLFW_REPEAT: {
						WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);//可以看一看glfwSetWindowUserPointer(m_Window, &m_Data)处的猜想

						KeyPressedEvent event(key, scancode, 1);
						data.EventCallback(event);
						break;
					}
					case GLFW_RELEASE: {
						WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);//可以看一看glfwSetWindowUserPointer(m_Window, &m_Data)处的猜想

						KeyReleasedEvent event(key, scancode);
						data.EventCallback(event);
						break;
					}
				}
			});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int codepoint)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				int scancode = glfwGetKeyScancode(codepoint);
				TextEvent event(codepoint, scancode);

				data.EventCallback(event);
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
		#pragma endregion

	}


	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		// 为了接收事件并证明程序未陷入死锁，GLFW 需要定期与窗口系统进行通信。只要窗口处于可见状态，就必须定期处理事件，通常在每一帧完成缓冲区交换后进行。
		// 处理待处理事件有两种方式：轮询和等待。这里采用事件轮询（Polling）机制，它仅处理当前已收到的事件并立即返回。
		// 对于像大多数游戏那样需要持续渲染的场景，这是最佳选择。
		// 相反，如果你只需要在接收到新输入时才更新画面，那么使用 glfwWaitEvents 会更好。
		// 它会阻塞并使线程进入休眠状态，直到至少接收到一个事件后才开始处理。这能节省大量的 CPU 资源，非常适合诸如各类编辑工具之类的应用。

		glfwSwapBuffers(m_Window);//交换前后台缓冲（可能是双缓冲方案）
		// 前台缓冲区用于显示当前图像，而后台缓冲区则用于执行渲染操作。
	}

	void WindowsWindow::Shutdown() {
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	WindowsWindow::~WindowsWindow() {
		Shutdown();
	}

	void WindowsWindow::SetVSync(bool enabled) 
	{ 
		//通过调节前后台缓冲的交替间隔（交换缓冲前所需等待的帧数）来实现垂直同步
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
