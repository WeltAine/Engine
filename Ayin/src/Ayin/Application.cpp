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
		//m_VertexArray = std::make_shared<VertexArray>(VertexArray::Create());//shared_ptr默认构造是啥都不敢，但是make方法会
		m_VertexArray.reset(VertexArray::Create());

		// VBO
		float vertices[3 * 3] = {
			0.0f, 0.5f, 0.0f,
			-0.5f, -0.5f, 0.0f,
			0.5f, -0.5f, 0.0f
		};
		m_VertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

		BufferLayout layout{ { {ShaderDataType::Float3, "a_Position"} } };
		m_VertexBuffer->SetLayout(layout);

		m_VertexArray->AddVertexBuffer(m_VertexBuffer);


		// EBO
		unsigned int indices[1 * 3] = {//不能用int
			0, 1, 2
		};
		m_IndexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		//glBindVertexArray(0);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


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

		//std::vector<Shader> vs = { {vertexShaderSrc, fragmentShaderSrc} };
		//Shader shader = { vertexShaderSrc, fragmentShaderSrc };
		// 这是一个很有意思的Bug，表现：当解除第一个注释时画面会闪过一个白色三角，然后就只显示背景；解除第二个注释却不会有任何影响
		// Bug稳定复现，大概率是一个逻辑问题
		// 两个注释的整个过程中都要经历构造和析构，那为何结果会如此不同，而且对窗口现象也逆向推理不出什么，经验太少了
		// 直接问AI
		// 注释一中产生了一个临时右值，在本行结束时被提前释放了（右值Shader触发析构，释放了上下文中的资源），但vs中是浅拷贝来的，还记录着ID（m_ProgramID）
		// 后面的m_Shader也占了这个ID对应的资源（因为vs产生的ID已经丢失了，所以此时m_Shader会再次被分配此ID）
		// 在本方法结尾vs被析构，其中元素（左值Shader）也触发析构，上下文里的ID对应的资源被释放（而该ID和m_Shader中的ID一致）

		m_Shader.reset(new Shader(vertexShaderSrc, fragmentShaderSrc));

		#pragma endregion
	};


	void Application::Run() {

		WindowResizeEvent e(1280, 720);

		while (m_Running)
		{
			glClear(GL_COLOR_BUFFER_BIT);

			// 图形渲染
			m_Shader->Bind();//在一些渲染API中，要求绑定VAO之前就必须有一个着色器，以保证布局相对应（OpenGL没有这个限制），所以我们写在开头位置
			m_VertexArray->Bind();
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