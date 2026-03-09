#include <Ayin.h>


class ExampleLayer : public Ayin::Layer {

public:
	ExampleLayer()
		:Ayin::Layer("Example"), 
		m_SceneCamera{ Ayin::Camera::CameraType::Perspective, {.perspectiveProp = {.NearPlaneDistance = 0.1f, .FarPlaneDistance = 100.0f, .FOV = 60.0f, .AspectRatio = 16.0f / 9}} }

	{

		#pragma region 基础渲染流程参考（这一部分也可以放到OnAttach中）

		Ayin::RenderCommand::Init();

		//Gen方法与Create方法的区别？？？

		// VAO
		//m_VertexArray = std::make_shared<VertexArray>(VertexArray::Create());//shared_ptr默认构造是啥都不敢，但是make方法会
		m_VertexArray.reset(Ayin::VertexArray::Create());

		// VBO
		float vertices[3 * 3] = {
			0.0f, 0.5f, 0.0f,
			-0.5f, -0.5f, 0.0f,
			0.5f, -0.5f, 0.0f
		};
		std::shared_ptr<Ayin::VertexBuffer> vertexBuffer(Ayin::VertexBuffer::Create(vertices, sizeof(vertices)));

		Ayin::BufferLayout layout{ { {Ayin::ShaderDataType::Float3, "a_Position"} } };
		vertexBuffer->SetLayout(layout);

		m_VertexArray->AddVertexBuffer(vertexBuffer);


		// EBO
		unsigned int indices[1 * 3] = {//不能用int
			0, 1, 2
		};
		std::shared_ptr<Ayin::IndexBuffer> indexBuffer(Ayin::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		indexBuffer.reset(Ayin::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

		m_VertexArray->SetIndexBuffer(indexBuffer);


		// 着色器
		std::string vertexShaderSrc = R"(
		#version 460 core
		layout(location = 0) in vec3 a_Position;
		
		out vec3 v_Position;

		uniform mat4 u_ProjectionViewMatrix;
		
		void main(){
			v_Position = a_Position;
			gl_Position = u_ProjectionViewMatrix * vec4(a_Position, 1.0f);
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

		//x std::vector<Shader> vs = { {vertexShaderSrc, fragmentShaderSrc} };
		//x Shader shader = { vertexShaderSrc, fragmentShaderSrc };
		//? 这是一个很有意思的Bug，表现：当解除第一个注释时画面会闪过一个白色三角，然后就只显示背景；解除第二个注释却不会有任何影响
		// Bug稳定复现，大概率是一个逻辑问题
		// 两个注释的整个过程中都要经历构造和析构，那为何结果会如此不同，而且对窗口现象也逆向推理不出什么，经验太少了
		// 直接问AI
		//! 首行代码中产生了一个临时右值，在本行结束时被提前释放了（右值Shader触发析构，释放了上下文中的资源），但vs中是浅拷贝来的，还记录着ID（m_ProgramID）
		//! 后面的m_Shader也占了这个ID对应的资源（因为vs产生的ID已经丢失了，所以此时m_Shader会再次被分配此ID）
		//! 在本方法结尾vs被析构，其中元素（左值Shader）也触发析构，上下文里的ID对应的资源被释放（而该ID和m_Shader中的ID一致）

		m_Shader.reset(new Ayin::Shader(vertexShaderSrc, fragmentShaderSrc));

		#pragma endregion

		m_SceneCamera.SetPosition(m_CameraPosition);
		m_SceneCamera.SetRotation(m_CameraRotation);

	
	}


	void OnUpdate() override {

		Ayin::RenderCommand::Clear();

		// 图形渲染
		Ayin::Renderer::BeginScene(m_SceneCamera);
		{
			Ayin::Renderer::Submit(m_Shader, m_VertexArray);//绑定shader并绘制VAO
		}
		Ayin::Renderer::EndScene();

	}

	void OnEvent(Ayin::Event& event) override {

		Ayin::EventDispatcher dispatcher(event);

		dispatcher.Dispatch<Ayin::KeyPressedEvent>(BIND_EVENT_FUN(ExampleLayer::OnAxisKeyPressed));
		dispatcher.Dispatch<Ayin::MouseMovedEvent>(BIND_EVENT_FUN(ExampleLayer::OnMouseMiddlePressed));
	}

	bool OnAxisKeyPressed(Ayin::KeyPressedEvent& event) {
		
		glm::vec3 distance{ 0.0f };

		//! 因为OpenGL对相机的可是朝向是z的负半轴，所以，这里的加减关系和直觉上是相反的
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_LEFT)) { distance.x -= m_MoveSpeed; };
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_RIGHT)) { distance.x += m_MoveSpeed; };
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_UP)) { distance.z -= m_MoveSpeed; };
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_DOWN)) { distance.z += m_MoveSpeed; };

		if (glm::length(distance) > 0.0f) {

			m_CameraPosition += glm::normalize(distance);

			m_SceneCamera.SetPosition(m_CameraPosition);

			AYIN_ERROR("Move");

		}

		AYIN_ERROR("{0}, {1}, {2}", m_CameraPosition.x, m_CameraPosition.y, m_CameraPosition.z);

		return false;
	}

	bool OnMouseMiddlePressed(Ayin::MouseMovedEvent& event) {

		auto [x, y] = Ayin::Input::GetMousePosition();

		x = (x - 640) / 16;
		y = (y - 360) / 16;

		static glm::vec3 lastRotation{ y, x, 0.0f };
		glm::vec3 rotation{ 0.0f };


		rotation = lastRotation - glm::vec3{ y, x, 0.0f };

		m_CameraRotation += rotation;

		m_SceneCamera.SetRotation(m_CameraRotation);

		lastRotation = { y, x, 0.0f };

		return false;

	}

private:

	std::shared_ptr<Ayin::Shader> m_Shader;				//着色器程序
	std::shared_ptr<Ayin::VertexArray> m_VertexArray;	//顶点数组

	Ayin::Camera m_SceneCamera;							//场景相机

	glm::vec3 m_CameraPosition{0.0f, 0.0f, 3.0f};		//相机位置
	float m_MoveSpeed = 0.1f;							//移动速度

	glm::vec3 m_CameraRotation{0.0f};					//相机度数
	float m_RotationSpeed = 1.0f;						//转动速度

};


class SandBox : public Ayin::Application {

public:
	SandBox() {

		PushLayer(new ExampleLayer());//! 这里我们对于层的概念更加清晰了一些——游戏循环中某一模块的逻辑

	};

	~SandBox() {};

};



Ayin::Application* Ayin::CreatApplication() {

	return new SandBox();

}