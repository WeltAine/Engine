#include "AyinPch.h"

#include "OpenGLContext.h"
#include <Glad/glad.h>

namespace Ayin {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		:m_WindowHandle(windowHandle)
	{
	}

	void OpenGLContext::Init()
	{
		 
		// 将 OpenGL 上下文设置为当前上下文
		// 在使用 OpenGL API 之前，必须先创建一个当前的 OpenGL 上下文（Context）。该上下文将一直保持当前状态，直到你切换到另一个上下文，或者该上下文所属的窗口被销毁。
		glfwMakeContextCurrent(m_WindowHandle);//将当前线程与窗口绑定，该线程所执行的gl执行都将由该窗口的OpenGL/Vulkan上下文处理。

		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);//glad的核心功能之一是记录所有的现代OpenGL驱动方法，以便我们使用（否则我们就需要自己用GLFW的辅助方法拉驱动，用驱动，应该也是有的，可能比较老？），但是查找驱动的方法由GLFW提供，因为窗口是GLFW创建的，它知道是什么操作系统，该如何访问驱动
		// 加载器需要GLFW绑定当前上下文（Current Context）后才能工作。
		// 因为，根据文档（如下）
		// 如果当前上下文支持该函数，此函数将返回指定的 OpenGL 或 OpenGL ES 核心函数或扩展函数的地址。
		// 调用线程必须具有当前的上下文。如果在没有当前上下文的情况下调用此函数，将触发 GLFW_NO_CURRENT_CONTEXT 错误。

		AYIN_CORE_ASSERT(status, "Failed to initialize GLAD");

		AYIN_CORE_INFO("OpenGL Info");
		AYIN_CORE_INFO("  Vdndor: {0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		AYIN_CORE_INFO("  Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
		AYIN_CORE_INFO("  Version: {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
		// spdlog中只允许只允许直接格式化 char*/const char*（char*/const char* 是唯一被业界公认的 “字符串指针”）这两类指针被直接格式化
		// 对其他所有非 char* 类型的指针（包括 unsigned char*、int*、void* 等），默认禁止直接格式化，触发 formatting of non-void pointers is disallowed 的静态断言
		// 这么做的原因是处于安全考虑，并采用了典型的 “宁严勿宽” 的安全策略
		// fmt 库并不是完全禁止查看指针地址，而是禁止默认、无感知地格式化普通指针 —— 核心是为了安全，同时也给你留了 “明确查看地址” 的合法方式
		// 假设你误写：int* p = new int(10); AYIN_INFO("{}", p); —— 因为你错误地以为 p 是字符串指针，实际是 int*
		// 直接输出会导致：
		// 把 int 类型的内存数据当成字符解析，输出乱码
		// 至因为没有 \0 结束符，程序读取越界内存，触发崩溃 / 段错误
		// spdlog::info("指针 p 的地址是：{:p}", p);是正确查看地址地方式，或者强制转换为void*（spdlog默认它是输出地址的意图）
	}

	void OpenGLContext::SwapBuffer()
	{
		glfwSwapBuffers(m_WindowHandle);//交换前后台缓冲（双缓冲方案）
		// 前台缓冲区用于显示当前图像，而后台缓冲区则用于执行渲染操作。
	}

}

