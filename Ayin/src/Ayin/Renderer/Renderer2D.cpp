#include <AyinPch.h>

#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity


#include "Ayin/Renderer/Renderer2D.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "Ayin/Renderer/RenderCommand.h"

namespace Ayin {


	Renderer2DStroage* Renderer2D::s_Data;

	void Renderer2D::Init() {
	
		s_Data = new Renderer2DStroage;

		s_Data->QuadVAO = Ayin::VertexArray::Create();

		//VBO
		float vertexs[] = {
			-0.5f, -0.5f,  0.0f,	0.0f, 0.0f,
			 0.5f, -0.5f,  0.0f,	1.0f, 0.0f,
			 0.5f,  0.5f,  0.0f,	1.0f, 1.0f,
			-0.5f,  0.5f,  0.0f,	0.0f, 1.0f
		};
		Ayin::Ref<Ayin::VertexBuffer> vbo = Ayin::VertexBuffer::Create(vertexs, sizeof(vertexs));
		vbo->SetLayout(Ayin::BufferLayout{
			Ayin::BufferElement{0, Ayin::ShaderDataType::Float3, "a_Position"},
			Ayin::BufferElement{1, Ayin::ShaderDataType::Float2, "a_UV"}
			});
		//EBO
		uint32_t indexs[] = {
			0, 3, 2,
			0, 2, 1
		};
		Ayin::Ref<Ayin::IndexBuffer> ebo = Ayin::IndexBuffer::Create(indexs, sizeof(indexs));
		//VAO
		s_Data->QuadVAO->AddVertexBuffer(vbo);
		s_Data->QuadVAO->SetIndexBuffer(ebo);


		//Shader
		s_Data->QuadShader = Ayin::Shader::Create("O:/CppProgram/Ayin/assets/shader/2D/QuadShader.glsl");
		s_Data->QuadTextureShader = Ayin::Shader::Create("O:/CppProgram/Ayin/assets/shader/2D/QuadTextureShader.glsl");


	};

	void Renderer2D::Shutdown() {
	
		delete s_Data;

	};

	void Renderer2D::BeginScene(const Camera& camera) {
	
		s_Data->QuadShader->SetMat4("u_ProjectionViewMatrix", camera.GetProjecttionViewMatrix());
		s_Data->QuadTextureShader->SetMat4("u_ProjectionViewMatrix", camera.GetProjecttionViewMatrix());

	};
	void Renderer2D::EndScene() {};


	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& size) {
	
		glm::mat4 transform{1.0f};
		transform = glm::translate(transform, position);
		transform = glm::scale(transform, size);

		glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));


		s_Data->QuadShader->Bind();
		s_Data->QuadShader->SetMat4("u_Transform", transform * pitch * yaw * roll);

		RenderCommand::DrawIndexed(s_Data->QuadVAO);
		s_Data->QuadShader->UnBind();

	};

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& size, const Ref<Texture2D>& texture) {
		
		glm::mat4 transform{ 1.0f };
		transform = glm::translate(transform, position);
		transform = glm::scale(transform, size);

		glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));


		s_Data->QuadTextureShader->Bind();
		texture->Bind(0);
		s_Data->QuadTextureShader->SetMat4("u_Transform", transform * pitch * yaw * roll);

		RenderCommand::DrawIndexed(s_Data->QuadVAO);
		s_Data->QuadTextureShader->UnBind();

	};

}

