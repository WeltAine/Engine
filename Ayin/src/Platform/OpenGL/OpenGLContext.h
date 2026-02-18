#pragma once

#include "Ayin/Renderer/GraphicsContext.h"
#include <GLFW/glfw3.h>

namespace Ayin {

	/// <summary>
	/// 记录上下文与其对应的输出窗口
	/// </summary>
	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);

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
		GLFWwindow* m_WindowHandle;
	};

}


