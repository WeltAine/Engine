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
		// 加载器需要绑定当前上下文（Current Context）才能工作。

		AYIN_CORE_ASSERT(status, "Failed to initialize GLAFD");

	}

	void OpenGLContext::SwapBuffer()
	{
		glfwSwapBuffers(m_WindowHandle);//交换前后台缓冲（双缓冲方案）
		// 前台缓冲区用于显示当前图像，而后台缓冲区则用于执行渲染操作。
	}

}

