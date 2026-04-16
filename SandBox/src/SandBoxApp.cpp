#include <Ayin.h>

// ---入口点--------------------------
#include "Ayin/Core/EntryPoint.h"
// -----------------------------------

#include <Platform/OpenGL/Buffer/OpenGLBuffer.h>

#include "SandBox2D.h"



class ExampleLayer : public Ayin::Layer {

public:
	ExampleLayer()
		:Ayin::Layer("Example"), 
		m_SceneCameraController{ {.Type = Ayin::Camera::CameraType::Perspective, .FOV = 60.0f, .AspectRatio = (16.0f/9.0f), .NearPlaneDistance = 0.1f, .FarPlaneDistance = 100.0f}}
	{

		#pragma region 基础渲染流程参考（这一部分也可以放到OnAttach中）

		//Gen方法与Create方法的区别？？？

		// VAO
		m_TriangleVertexArray = Ayin::VertexArray::Create();
		m_SquareVertexArray = Ayin::VertexArray::Create();

		// VBO
		{
			float vertices[3 * 3] = {
				0.0f, 0.5f, 0.0f,
				-0.5f, -0.5f, 0.0f,
				0.5f, -0.5f, 0.0f
			};
			Ayin::Ref<Ayin::VertexBuffer> vertexBuffer(Ayin::VertexBuffer::Create(vertices, sizeof(vertices)));

			Ayin::BufferLayout layout{ { {0, Ayin::ShaderDataType::Float3, "a_Position"} } };
			vertexBuffer->SetLayout(layout);

			m_TriangleVertexArray->AddVertexBuffer(vertexBuffer);

		}

		{
			float vertices[4 * (3 + 2)] = {
				-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,
				 0.5f, -0.5f, 0.0f,		1.0f, 0.0f,
				-0.5f,  0.5f, 0.0f,		0.0f, 1.0f,
				 0.5f,  0.5f, 0.0f,		1.0f, 1.0f
			};
			Ayin::Ref<Ayin::VertexBuffer> vertexBuffer(Ayin::VertexBuffer::Create(vertices, sizeof(vertices)));

			Ayin::BufferLayout layout{ { Ayin::BufferElement{0, Ayin::ShaderDataType::Float3, "a_Position"}, 
										 Ayin::BufferElement{1, Ayin::ShaderDataType::Float2, "a_UV"	  }}};
			vertexBuffer->SetLayout(layout);

			m_SquareVertexArray->AddVertexBuffer(vertexBuffer);

		}



		// EBO
		{
			unsigned int indices[1 * 3] = {//不能用int
				0, 1, 2
			};
			Ayin::Ref<Ayin::IndexBuffer> indexBuffer(Ayin::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
			indexBuffer = (Ayin::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

			m_TriangleVertexArray->SetIndexBuffer(indexBuffer);

		}

		{
			unsigned int indices[2 * 3] = {//不能用int
				0, 1, 3,
				3, 2, 0
			};
			Ayin::Ref<Ayin::IndexBuffer> indexBuffer(Ayin::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
			indexBuffer = (Ayin::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

			m_SquareVertexArray->SetIndexBuffer(indexBuffer);

		}

		// 着色器
		m_ShaderLibrary.Load("O:/CppProgram/Ayin/assets/shader/shader.glsl");
		m_TextureShader = Ayin::Shader::Create("O:/CppProgram/Ayin/assets/shader/TextureShader.glsl");


		//纹理
		m_Texture = Ayin::Texture2D::Create("O:/CppProgram/Ayin/assets/textures/texture.png");//不支持中文路径

		m_BlendTexture = Ayin::Texture2D::Create("O:/CppProgram/Ayin/assets/textures/blendTexture.png");
		m_TextureShader->SetInt("ourTexture", 0);
		//! sampler2D本质是一个int对应纹理单元的槽位（你也可以在shader代码中用location来指定）


		#pragma endregion

		//Mode(使用统一缓冲,UBO)
		{
			glm::mat4 translate = glm::translate(glm::identity<glm::mat4>(), glm::vec3{ 0.0f });
			m_UBO = Ayin::UniformBuffer::Create(static_cast<void*>(glm::value_ptr(translate)), sizeof(translate));
			m_UBO->SetBindingIndexs({1});
			m_UBO->SetLayout(Ayin::UniformLayout{ "TransformBlock", { Ayin::UniformElement{Ayin::ShaderDataType::Mat4, "t_Transform"}} });
		}

		{
			glm::mat4 translate = glm::translate(glm::identity<glm::mat4>(), glm::vec3{ 0.0f });
			m_BlendUBO = Ayin::UniformBuffer::Create(static_cast<void*>(glm::value_ptr(translate)), sizeof(translate));
			m_BlendUBO->SetBindingIndexs({1});
			m_BlendUBO->SetLayout(Ayin::UniformLayout{ "TransformBlock", { Ayin::UniformElement{Ayin::ShaderDataType::Mat4, "t_Transform"}} });
		}

		
	
	}


	virtual void OnUpdate( Ayin::Timestep deltaTime ) override {

		//OnAxisKeyPressed(deltaTime);
		//OnMouseMoved(deltaTime);
		m_SceneCameraController.OnUpdate(deltaTime);


		Ayin::RenderCommand::Clear();

		// 图形渲染
		Ayin::Renderer::BeginScene(m_SceneCameraController.GetCamera());
		{
			{
				m_Transform = glm::translate(m_Transform, glm::vec3{ 0.1f * deltaTime, 0.0f, 0.0f });
				std::dynamic_pointer_cast<Ayin::OpenGLUniformBuffer>(m_UBO)->UploadMat4("t_Transform", m_Transform);
			}

			{
				glm::mat4 transform = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f * deltaTime, 0.0f, 0.0f });
				std::dynamic_pointer_cast<Ayin::OpenGLUniformBuffer>(m_BlendUBO)->UploadMat4("t_Transform", transform);
			}



			//std::dynamic_pointer_cast<Ayin::OpenGLShader>(m_Shader)->UploadUniformFloat3("colorOffset", m_ColorOffset);
			//Ayin::Renderer::Submit(m_Shader, m_TriangleVertexArray, m_UBO);//绑定shader并绘制VAO

			m_Texture->Bind(0);
			m_UBO->Bind();
			m_UBO->SetBindingIndexs({1});
			Ayin::Renderer::Submit(m_ShaderLibrary.Get("shader"), m_SquareVertexArray);


			m_TextureShader->SetFloat3("colorOffset", m_ColorOffset);
			m_BlendTexture->Bind(0);
			m_BlendUBO->Bind();
			m_BlendUBO->SetBindingIndexs({1});
			Ayin::Renderer::Submit(m_TextureShader, m_SquareVertexArray);


		}
		Ayin::Renderer::EndScene();

	}

	virtual void OnEvent(Ayin::Event& e) override {
		m_SceneCameraController.OnEvent(e);
	}

	virtual void OnImGuiRender() override {

		ImGui::Begin("Color Offset");
			ImGui::ColorEdit3("color", glm::value_ptr(m_ColorOffset));
		ImGui::End();

	}



private:

	Ayin::ShaderLibrary m_ShaderLibrary;
	Ayin::Ref<Ayin::Shader> m_TextureShader;										//着色器程序
	Ayin::Ref<Ayin::Texture2D> m_Texture, m_BlendTexture;							//纹理
	Ayin::Ref<Ayin::UniformBuffer> m_UBO, m_BlendUBO;								//统一变量缓冲
	glm::mat4 m_Transform{1.0f};	// mode测试
	glm::vec3 m_ColorOffset{ 0.0f };// 其余uniform参数设置
	Ayin::Ref<Ayin::VertexArray> m_TriangleVertexArray, m_SquareVertexArray;		//顶点数组

	Ayin::CameraController m_SceneCameraController;									//场景相机


};


class SandBox : public Ayin::Application {

public:
	SandBox() {

		//PushLayer(new ExampleLayer{});//! 这里我们对于层的概念更加清晰了一些——游戏循环中某一模块的逻辑
		PushLayer(new SandBox2D{});

	};

	~SandBox() {};

};



Ayin::Application* Ayin::CreatApplication() {

	return new SandBox();

}