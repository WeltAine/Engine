#include <AyinPch.h>

#include "Ayin/Renderer/RenderCommand.h"
#include "Ayin/Renderer/Renderer2D.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity

namespace Ayin {


	Renderer2D_Data Renderer2D::s_Data;

	void Renderer2D::Init() {
	
		s_Data.m_QuadVAO = VertexArray::Create();

		//基础VBO
		float vertexs[] = {
			-0.5f, -0.5f,  0.0f,	0.0f, 0.0f,
			 0.5f, -0.5f,  0.0f,	1.0f, 0.0f,
			 0.5f,  0.5f,  0.0f,	1.0f, 1.0f,
			-0.5f,  0.5f,  0.0f,	0.0f, 1.0f
		};
		Ref<VertexBuffer> vbo = VertexBuffer::Create(vertexs, sizeof(vertexs));
		vbo->SetLayout(BufferLayout{
			BufferElement{0, ShaderDataType::Float3, "a_VertexPosition"},
			BufferElement{1, ShaderDataType::Float2, "a_VertexUV"}
			});
		//基础EBO
		uint32_t indexs[] = {
			0, 3, 2,
			0, 2, 1
		};
		Ref<IndexBuffer> ebo = IndexBuffer::Create(indexs, sizeof(indexs) / sizeof(uint32_t));
		//VAO
		s_Data.m_QuadVAO->AddVertexBuffer(vbo);
		s_Data.m_QuadVAO->SetIndexBuffer(ebo);

		//QuadVBO
		s_Data.m_QuadBuffer = VertexBuffer::Create(Renderer2D_Data::MaxQuads * sizeof(Quad));
		s_Data.m_QuadBuffer->SetLayout(BufferLayout{
			BufferElement {2, ShaderDataType::Float3, "a_Position"},
			BufferElement {3, ShaderDataType::Float3, "a_Rotation"},
			BufferElement {4, ShaderDataType::Float3, "a_Size"},
			BufferElement {5, ShaderDataType::Float4, "a_Color"},
			BufferElement {6, ShaderDataType::Float2, "a_TilingFactor"},
			BufferElement {7, ShaderDataType::Int, "a_TextureIndex"}
			});
		s_Data.m_QuadVAO->AddVertexBuffer(s_Data.m_QuadBuffer, 1);

		//批处理数据准备
		s_Data.m_QuadBufferBase = new Quad[Renderer2D_Data::MaxQuads];
		s_Data.m_QuadBufferPtr = s_Data.m_QuadBufferBase;

		//Shader
		s_Data.m_QuadShader = Shader::Create("O:/CppProgram/Ayin/assets/shader/2D/QuadShader.glsl");

		//默认Texture
		s_Data.m_WhiteTexture = Texture2D::Create(100, 100);

		s_Data.m_TextureSlots[0] = s_Data.m_WhiteTexture;

		int32_t samplers[32];
		for (int i = 0; i < Renderer2D_Data::MaxTextureSlots; i++) {
			samplers[i] = i;
		}
		s_Data.m_QuadShader->SetIntArray("u_Textures", samplers, Renderer2D_Data::MaxTextureSlots);

		//? 如果你之前将sampler2D简单视为 int 的话，一定会对 samplers 和u _Texture[32]感到困惑
		//! 不透明类型：在 GLSL 中， sampler2D 属于不透明类型。它既不是指针，也不是简单的整数，而是 GPU 用于访问纹理单元的句柄。
		//! 非法类型转换：你正尝试将整数（ v_TextureIndex ）直接“构造”或“强制转换”为 sampler2D 。虽然这种语法与 C++ 相似，但在 GLSL 中，仅在启用无绑定纹理（Bindless Textures，通过 GL_ARB_bindless_texture 等扩展）的情况下，该语法才合法。
		//  如果不使用samplers和u_Texture[32] 
		//! 由于未启用 bindless texture 扩展，驱动在尝试链接时，发现无法在运行时将随机顶点属性转换为纹理句柄，随即触发了空指针解引用，最终导致程序崩溃。
	};

	void Renderer2D::Shutdown() {
	
		delete[] s_Data.m_QuadBufferBase;

	};

	void Renderer2D::BeginScene(const Camera& camera) {
	
		s_Data.m_QuadShader->SetMat4("u_ProjectionViewMatrix", camera.GetProjecttionViewMatrix());


		s_Data.m_QuadCount = 0;
		s_Data.m_TextureSlotInsertIndex = 1;
		s_Data.m_QuadBufferPtr = s_Data.m_QuadBufferBase;

	};
	void Renderer2D::EndScene() {
		
		size_t dataSize = (s_Data.m_QuadBufferPtr - s_Data.m_QuadBufferBase) * sizeof(Quad);
		s_Data.m_QuadBuffer->SetData(s_Data.m_QuadBufferBase, dataSize);
		
		Flush();

	};


	void Renderer2D::Flush() {

		for (int index = 0; index < s_Data.m_TextureSlotInsertIndex; index++) {
			s_Data.m_TextureSlots[index]->Bind(index);
		}

		s_Data.m_QuadShader->Bind();

		RenderCommand::Draw2DIndexed(s_Data.m_QuadVAO, s_Data.m_QuadCount);

		s_Data.m_QuadShader->UnBind();

	}
	

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& size, const glm::vec4& color) {
	
		//glm::mat4 transform{1.0f};
		//transform = glm::translate(transform, position);
		//transform = glm::scale(transform, size);

		//glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		//glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		//glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		//s_Data->QuadShader->SetMat4("u_Transform", transform * pitch * yaw * roll);

		//s_Data->QuadShader->SetFloat4("u_Color", color);
		//s_Data->QuadShader->SetFloat2("u_TilingFactor", glm::vec2{1.0f, 1.0f});

		//s_Data->QuadShader->Bind();
		//s_Data->WhiteTexture->Bind(0);

		//RenderCommand::DrawIndexed(s_Data->QuadVAO);

		//s_Data->WhiteTexture->UnBind();
		//s_Data->QuadShader->UnBind();

		s_Data.m_QuadBufferPtr->Position = position;
		s_Data.m_QuadBufferPtr->Rotation = rotation;
		s_Data.m_QuadBufferPtr->Size = size;
		s_Data.m_QuadBufferPtr->Color = color;

		
		s_Data.m_QuadBufferPtr++;

		s_Data.m_QuadCount++;

	};

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor) {
		
		//glm::mat4 transform{ 1.0f };
		//transform = glm::translate(transform, position);
		//transform = glm::scale(transform, size);

		//glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		//glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		//glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		//s_Data->QuadShader->SetMat4("u_Transform", transform * pitch * yaw * roll);

		//s_Data->QuadShader->SetFloat4("u_Color", glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
		//s_Data->QuadShader->SetFloat2("u_TilingFactor", tilingFactor);

		//s_Data->QuadShader->Bind();
		//texture->Bind(0);

		//RenderCommand::DrawIndexed(s_Data->QuadVAO);

		//texture->UnBind();
		//s_Data->QuadShader->UnBind();

		int textureIndex = 0;
		for (int index = 0; index < s_Data.m_TextureSlotInsertIndex; index++) {

			if (*(s_Data.m_TextureSlots[index].get()) == *(texture.get())) {
				textureIndex = index;
				break;
			}

		}

		if (textureIndex == 0) {
			texture->Bind(s_Data.m_TextureSlotInsertIndex);
			s_Data.m_TextureSlots[s_Data.m_TextureSlotInsertIndex] = texture;
			textureIndex = s_Data.m_TextureSlotInsertIndex++;
		}

		s_Data.m_QuadBufferPtr->Position = position;
		s_Data.m_QuadBufferPtr->Rotation = rotation;
		s_Data.m_QuadBufferPtr->Size = size;
		s_Data.m_QuadBufferPtr->TilingFactor = tilingFactor;
		s_Data.m_QuadBufferPtr->TextureIndex = textureIndex;

		s_Data.m_QuadBufferPtr++;

		s_Data.m_QuadCount++;


	};

}

