#include "AyinPch.h"

#include "Application.h"

#include "spdlog/fmt/bundled/format.h"

#include "Ayin/Events/ApplicationEvent.h"
#include "Ayin/Log.h"

#include "Ayin/Input.h"

#include "Ayin/Renderer/Buffer.h"


#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Ayin {

	Application* Application::s_Instance = nullptr;

	/// <summary>
	/// 创建窗口，单例，层栈以及为窗口的人造闭包配置事件函指
	/// </summary>
	Application::Application() 
	{
		AYIN_ASSERT(!s_Instance, "Application already exists!");//断言，防止破坏单例
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());

		//当窗口发生事件时（通过GLFW），窗口类会调用OnEvent回调，由我们来处理，Application成了中介者，而窗口成为了组件，这道函指则是组件与中介之间的沟通桥梁
		m_Window->SetEventCallback(BIND_EVENT_FUN(Application::OnEvent));//通过

		//栈层使用默认构造
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);//暂时没有空间清理的过程，可能泄露（在拆出层时）,感觉该层的所有权有写模糊啊


		#pragma region 基础渲染流程参考（之后会逐步抽象过程中所用到的部件）

		//Gen方法与Create方法的区别？？？

		// VAO
		glGenVertexArrays(1, &m_VertexArray);
		glBindVertexArray(m_VertexArray);

		// VBO
		float vertices[3 * 3] = {
			0.0f, 0.5f, 0.0f,
			-0.5f, -0.5f, 0.0f,
			0.5f, -0.5f, 0.0f
		};
		m_VertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// EBO
		unsigned int indices[1 * 3] = {//不能用int
			0, 1, 2
		};
		m_IndexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


		// 着色器
		std::string vertexShaderSrc = R"(
		#version 460 core
		layout(location = 0) in vec3 a_Position;
		
		out vec3 v_Position;
		
		void main(){
			v_Position = a_Position;
			gl_Position = vec4(a_Position, 1.0f);
		}
		)";

		std::string fragmentShaderSrc = R"(
		#version 460 core
		in vec3 v_Position;
		
		out vec4 color;
		
		void main(){
			color = vec4(v_Position * 0.5f + 0.5f, 1.0f);
		}
		)";

		std::vector<Shader> vs = { {vertexShaderSrc, fragmentShaderSrc} };

		m_Shader.reset(new Shader(vertexShaderSrc, fragmentShaderSrc));

		#pragma endregion
	};

	Application::~Application() {
		glDeleteVertexArrays(1, &m_VertexArray);
	};

	void Application::Run() {

		WindowResizeEvent e(1280, 720);

		while (m_Running)
		{
			glClear(GL_COLOR_BUFFER_BIT);

			// 图形渲染
			m_Shader->Bind();//在一些渲染API中，要求绑定VAO之前就必须有一个着色器，以保证布局相对应（OpenGL没有这个限制），所以我们写在开头位置

			glBindVertexArray(m_VertexArray);
			glDrawElements(GL_TRIANGLES, m_IndexBuffer->GetCount(), GL_UNSIGNED_INT, (void*)0);//即使glClearColor在后面的ImGuilayer中，也不影响结果
			// OpenGL 明确规定，glDrawElements 的第三个参数（索引类型）只能是以下三种：
			//		GL_UNSIGNED_BYTE（对应 C 类型：unsigned char）
			//		GL_UNSIGNED_SHORT（对应 C 类型：unsigned short）
			//		GL_UNSIGNED_INT（对应 C 类型：unsigned int）
			//		GL_INT 并不是合法的参数值，绝大多数 OpenGL 驱动会直接忽略这个绘制调用，或触发错误。

			// 层更新
			for (Layer* layer : m_LayerStack) {//？？？只要有begin和end就可以foreack么，查查背后的语法糖
				layer->OnUpdate();
			}

			// 渲染ImGui（之后会单独放到渲染线程上，所以不会在Layer的OnUpdate中执行）
			// m_ImGuiLauer的Begin和End别的层也能用么？？？
			// 这里应该接受的是上一帧的情况
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack) {
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();
			
			// 窗口更新
			m_Window->OnUpdate();

		}
	}

	void Application::OnEvent(Event& e) {

		//输出日志（除关闭以外的事件先这么处理）
		AYIN_CORE_TRACE("{0}", e);

		EventDispatcher dispatcher(e);

		//关闭事件
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FUN(Application::OnWindowClose));


		//从后往前逐层通知事件
		for (auto iterator = m_LayerStack.end(); iterator != m_LayerStack.begin(); ) {

			(*--iterator)->OnEvent(e);
			if (e.handled) {
				break;
			}
		}

	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {

		m_Running = false;
		return true;
	}


	void Application::PushLayer(Layer* layer) {
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overLayer) {
		m_LayerStack.PushOverlay(overLayer);
	}

}