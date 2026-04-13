#include "SandBox2D.h"


SandBox2D::SandBox2D()
	:Ayin::Layer("SandBox2DLayer"), m_CamreaController(Ayin::CameraProp{.Type = Ayin::Camera::CameraType::Orthogonal})
{};


void SandBox2D::OnAttach() {

	m_SquareVAO = Ayin::VertexArray::Create();

	//VBO
	float vertexs[] = {
		-0.5f, -0.5f,  0.0f,
		 0.5f, -0.5f,  0.0f,
		 0.5f,  0.5f,  0.0f,
		-0.5f,  0.5f,  0.0f
	};
	Ayin::Ref<Ayin::VertexBuffer> vbo = Ayin::VertexBuffer::Create(vertexs, sizeof(vertexs));
	vbo->SetLayout(Ayin::BufferLayout{
		Ayin::BufferElement{0, Ayin::ShaderDataType::Float3, "a_Position"}
		});
	//EBO
	uint32_t indexs[] = {
		0, 3, 2,
		0, 2, 1
	};
	Ayin::Ref<Ayin::IndexBuffer> ebo = Ayin::IndexBuffer::Create(indexs, sizeof(indexs));
	//VAO
	m_SquareVAO->AddVertexBuffer(vbo);
	m_SquareVAO->SetIndexBuffer(ebo);


	//Shader
	m_SquareShader = Ayin::Shader::Create("O:/CppProgram/Ayin/assets/shader/SquareShader.glsl");


};
void SandBox2D::OnDetach() {};

void SandBox2D::OnUpdate(Ayin::Timestep deltaTime) {

	m_CamreaController.OnUpdate(deltaTime);

	Ayin::Renderer::BeginScene(m_CamreaController.GetCamera());

		Ayin::Renderer::Submit(m_SquareShader, m_SquareVAO, glm::identity<glm::mat4>());

	Ayin::Renderer::EndScene();

};

void SandBox2D::OnImGuiRender() {
	
};

void SandBox2D::OnEvent(Ayin::Event& event) {
	m_CamreaController.OnEvent(event);
};
