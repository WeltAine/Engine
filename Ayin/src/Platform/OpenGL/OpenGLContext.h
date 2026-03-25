#pragma once

#include "Ayin/Window.h"
#include "Ayin/Renderer/GraphicsContext.h"
#include <GLFW/glfw3.h>

namespace Ayin {

	/// <summary>
	/// OpenGL上下文，使用OpenGL相关驱动来进行绘制，通过glad加载函数指针给glfw，是程序能够在窗口上绘制的关键
	/// 记录上下文与其对应的输出窗口
	/// </summary>
	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(Window* windowHandle);

		// 通过 GraphicsContext 继承
		
		/// <summary>
		/// 将本上下文对应的人窗口作为渲染目标
		/// </summary>
		virtual void Init() override;

		/// <summary>
		/// 交换帧缓冲
		/// </summary>
		virtual void SwapBuffer() override;


	private:
		//GLFWwindow* m_WindowHandle;
		Window* m_WindowHandle;
	};

}


