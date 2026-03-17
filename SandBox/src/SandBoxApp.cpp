#include <Ayin.h>
#include <Platform/OpenGL/OpenGLShader.h>



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
		

		layout(std140, binding = 1) uniform TransformBlock{
			mat4 t_Position;
		};

		void main(){
			v_Position = a_Position;
			gl_Position = u_ProjectionViewMatrix * t_Position * vec4(a_Position, 1.0f);
		}
		)";

		std::string fragmentShaderSrc = R"(
		#version 460 core
		in vec3 v_Position;
		
		out vec4 color;

		uniform vec3 colorOffset;
		
		void main(){
			color = vec4(v_Position * 0.5f + 0.5f, 1.0f) + vec4(colorOffset, 1.0f);
		}
		)";


		m_Shader.reset(Ayin::Shader::Create(vertexShaderSrc, fragmentShaderSrc));

		#pragma endregion

		glm::mat4 translate = glm::translate(glm::identity<glm::mat4>(), glm::vec3{0.0f});
		m_UBO.reset(Ayin::UniformBuffer::Create(static_cast<void*>(glm::value_ptr(translate)), sizeof(translate)));
		m_UBO->SetIndex(1);
		m_UBO->SetLayout(Ayin::UniformLayout{ "TransformBlock", { Ayin::UniformElement{Ayin::ShaderDataType::Mat4, "t_Position"}} });

		m_SceneCamera.SetPosition(m_CameraPosition);
		m_SceneCamera.SetRotation(m_CameraRotation);
		 
	
	}


	virtual void OnUpdate( Ayin::Timestep deltaTime ) override {

		//OnAxisKeyPressed(deltaTime);
		//OnMouseMoved(deltaTime);


		Ayin::RenderCommand::Clear();

		// 图形渲染
		Ayin::Renderer::BeginScene(m_SceneCamera);
		{
			m_Transform = glm::translate(m_Transform, glm::vec3{ 0.1f * deltaTime, 0.0f, 0.0f });
			m_UBO->Set(
				"t_Position",
				static_cast<void*>(glm::value_ptr(m_Transform))
			);


			std::dynamic_pointer_cast<Ayin::OpenGLShader>(m_Shader)->UploadUniformFloat3("colorOffset", m_ColorOffset);
			Ayin::Renderer::Submit(m_Shader, m_VertexArray, m_UBO);//绑定shader并绘制VAO
		}
		Ayin::Renderer::EndScene();

	}

	virtual void OnImGuiRender() override {

		ImGui::Begin("Color Offset");
			ImGui::ColorEdit3("color", glm::value_ptr(m_ColorOffset));
		ImGui::End();

	}


	void OnAxisKeyPressed( Ayin::Timestep deltaTime ) {
		//原点-》平移量 -》逆矩阵

		glm::vec3 distance{ 0.0f };

		//! 因为OpenGL对相机的可是朝向是z的负半轴，所以，这里的加减关系和直觉上是相反的
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_LEFT)) { distance.x -= m_MoveSpeed * deltaTime; };
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_RIGHT)) { distance.x += m_MoveSpeed * deltaTime; };
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_UP)) { distance.z -= m_MoveSpeed * deltaTime; };
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_DOWN)) { distance.z += m_MoveSpeed * deltaTime; };

		if (glm::length(distance) > 0.0f) {

			//! 原点 -> 平移量 -> 逆矩阵
			//让移动基于相机的本地坐标，当然GetRotationMatrix()是我临时加的，之后可能会移动到Transform中
			distance = m_SceneCamera.GetRotationMatrix() * glm::translate(glm::identity<glm::mat4>(), distance) * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f};

			m_CameraPosition += glm::normalize(distance);
			AYIN_ERROR("direction:{0}, {1}, {2}", distance.x, distance.y, distance.z);


			m_SceneCamera.SetPosition(m_CameraPosition);

			//AYIN_ERROR("Move");
			//AYIN_ERROR("{0}, {1}, {2}", m_CameraRotation.x, m_CameraRotation.y, m_CameraRotation.z);

		}

	}

	void OnMouseMoved( Ayin::Timestep deltaTime ) {

		auto [x, y] = Ayin::Input::GetMousePosition();

		x = (x - 640);
		y = (y - 360);

		static glm::vec3 lastRotation{ y, x, 0.0f };
		glm::vec3 rotation{ 0.0f };


		rotation = glm::vec3{ y, x, 0.0f };


		if (glm::length(rotation) > 30.0f) {

			m_CameraRotation -= glm::normalize(rotation) * m_RotationSpeed  * float(deltaTime);

			m_SceneCamera.SetRotation(m_CameraRotation);

			lastRotation = { y, x, 0.0f };

			AYIN_ERROR("Rotate");
			AYIN_ERROR("{0}, {1}, {2}", m_CameraRotation.x, m_CameraRotation.y, m_CameraRotation.z);

		}


	}

private:

	std::shared_ptr<Ayin::Shader> m_Shader;				//着色器程序
	std::shared_ptr<Ayin::UniformBuffer> m_UBO;
	glm::mat4 m_Transform{1.0f};
	glm::vec3 m_ColorOffset{ 0.0f };
	std::shared_ptr<Ayin::VertexArray> m_VertexArray;	//顶点数组

	Ayin::Camera m_SceneCamera;							//场景相机

	glm::vec3 m_CameraPosition{0.0f, 0.0f, 3.0f};		//相机位置
	float m_MoveSpeed = 0.1f;							//移动速度

	glm::vec3 m_CameraRotation{0.0f};					//相机度数
	float m_RotationSpeed = 30.0f;						//转动速度

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