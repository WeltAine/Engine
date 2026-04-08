#include "AyinPch.h"
#include "OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace Ayin{

	void OpenGLRendererAPI::Init() {

		//混合
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}

	void OpenGLRendererAPI::SetViewport(int x, int y, int width, int height) {
		glViewport(0, 0, width, height);
		//x 该API只会显示相机能看到的东西，改变的只是在窗口上的大小
		//x 这意味着单纯的拉伸窗口只会放大相机的照片尺寸而已，不会扩展视野，我们还需要调整相机的参数
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) const
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear() const
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray) const
	{
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, (void*)0);
		// OpenGL 明确规定，glDrawElements 的第三个参数（索引类型）只能是以下三种：
		//		GL_UNSIGNED_BYTE（对应 C 类型：unsigned char）
		//		GL_UNSIGNED_SHORT（对应 C 类型：unsigned short）
		//		GL_UNSIGNED_INT（对应 C 类型：unsigned int）
		//		GL_INT 并不是合法的参数值，绝大多数 OpenGL 驱动会直接忽略这个绘制调用，或触发错误。
	}

}

