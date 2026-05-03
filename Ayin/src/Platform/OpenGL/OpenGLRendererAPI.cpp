#include "AyinPch.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace Ayin{


	static void OpenGLMessageCallback(
		GLenum source, GLenum type, GLuint id, GLenum severity, //事件（消息）来源，消息类型，消息ID（可自定义），严重等级
		GLsizei length, const GLchar* message,					//字符串长度（为-1则一直读到null），消息字符串
		const void* userParam)									//用户指针（类似Ayin::Window类中的用户数据指针）
	{
	
		switch (severity) {

			case(GL_DEBUG_SEVERITY_HIGH):			AYIN_CORE_CRITICAL(message); return;
			case(GL_DEBUG_SEVERITY_MEDIUM):			AYIN_CORE_ERROR(message); return;
			case(GL_DEBUG_SEVERITY_LOW):			AYIN_CORE_WARN(message); return;
			case(GL_DEBUG_SEVERITY_NOTIFICATION):	AYIN_CORE_TRACE(message); return;

		}

	}
	// OpenGL提供的调试是将消息（或者说是事件触发时产生的记录）存储到OpenGL自己的日志缓冲中，而非直接输出到屏幕上，我们需要插入自己的回调来输出
	// 消息由特定部分组成——来源，类别，严重情况，消息id（有点像消息字符串的Key），严重等级，消息
	// 相关调试API
	// void glDebugMessageCallback(DEBUGPROC callback, const void *userParam)																设置回调
	// void glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)			消息过滤
	// void glDebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf)					插入（产生）一条自定义消息
	// void glObjectLabel(GLenum identifier, GLuint name, GLsizei length, const char *label)												对象赋名（在RenderDoc中就可以看到为对象设置的名字而非对象id）
	// glObjectPtrLabel和glGetObjectPtrLabel																								不知道
	// void glPushDebugGroup(GLenum source, GLuint id, GLsizei length, const char *message);												栈过程（组）开启消息（我们可以把一段代码逻辑包装为一个“组”，组开启时发出消息）
	// void glPopDebugGroup(void);																											栈过程（组）结束
	//GLuint glGetDebugMessageLog(GLuint count, GLsizei bufSize, GLenum *sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog);	从日志缓冲中获取消息，获取的消息将其各个部分填入参数所指向数组中，返回实际获取量



	void OpenGLRendererAPI::Init() {

		AYIN_PROFILE_FUNCTION();

		//混合
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);

#if AYIN_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

		glDebugMessageCallback(OpenGLMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

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

	void OpenGLRendererAPI::Draw2DIndexed(const Ref<VertexArray>& vertexArray, int count) const {

		if (count <= 1) {
			glDrawElementsInstanced(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, (void*)0, count);
			return;
		}

		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, (void*)0);

	}


}

