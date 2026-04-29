#include <AyinPch.h>

#include "Ayin/Renderer/RenderCommand.h"

#include "Ayin/Renderer/VertexArray.h"
#include "Ayin/Renderer/Shader.h"
#include "Ayin/Renderer/Camera.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity

#include "Ayin/Renderer/Renderer2D.h"


namespace Ayin {


	struct Quad {

		glm::vec3 Position;
		glm::vec3 Rotation;
		glm::vec3 Size;
		glm::vec4 Color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };

		glm::vec2 TilingFactor = glm::vec2{ 1.0f, 1.0f };
		int TextureIndex = 0;									//使用默认纹理	
		//? 一般来说这里应该使用float的，因为一些历史问题，老API无法使用int，不过现在有了，我么修改了VAO的AddVertexBuffer，以适配int
		//! Shader中int和float是可以转换的（不过好像有版本要求）
	};

	//! 关于GPU对寄存器的分配
	//! GPU 寄存器的分配并非逐个进行，而是以块（Block）或粒度（Granule）为单位进行分配的。一个块中有n个寄存器（假设32个）
	//! 工作原理：想象一个 GPU，其每个流式多处理器（SM）拥有 65,536 个寄存器。如果你的着色器仅占用 32 个寄存器，那么该 GPU 就能同时运行 2,048 个线程，从而达到 100% 的占用率。
	//! 性能悬崖：如果仅仅因为增加了一个 uint64_t  就导致编译器使用的寄存器数量从 32 个增加到 33 个，GPU 所能容纳的线程数可能会瞬间骤降至 1,024 个。
	//! 代码中一个微小的改动（仅增加一个 64 位变量）就可能导致 GPU 隐藏延迟的能力骤降 50%。这是因为占用率并非线性下降，而是呈巨大的阶梯状离散式下降。
	//! 而且寄存器中数据也需要对齐，则就可能会造成空隙，实际占用可能超过上述简单评估


	struct Renderer2D_Data {

		static const  uint32_t MaxQuads = 20000;
		static constexpr uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> QuadVAO;			//默认矩形
		Ref<VertexBuffer> QuadBatchBuffer;	//矩形数据缓冲对象（模型频率）
		Quad* QuadBatchBufferBase = nullptr;//矩阵数据（起始位置）
		Quad* QuadBatchBufferPtr = nullptr; //矩阵数据（当前写入位置）
		int QuadCount = 0;

		Ref<Shader> QuadShader;				//默认Shader

		Ref<Texture2D> WhiteTexture;		//默认纹理

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;	//纹理单元与其纹理管理
		uint32_t TextureSlotInsertIndex = 1;						//纹理可插入位置


		Renderer2D::Statistics Stats;

	};

	static Renderer2D_Data s_Data;



	void Renderer2D::Init() {

		AYIN_PROFILE_FUNCTION();
	
		s_Data.QuadVAO = VertexArray::Create();

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
		s_Data.QuadVAO->AddVertexBuffer(vbo);
		s_Data.QuadVAO->SetIndexBuffer(ebo);

		//QuadVBO
		s_Data.QuadBatchBuffer = VertexBuffer::Create(Renderer2D_Data::MaxQuads * sizeof(Quad));
		s_Data.QuadBatchBuffer->SetLayout(BufferLayout{
			BufferElement {2, ShaderDataType::Float3, "a_Position"},
			BufferElement {3, ShaderDataType::Float3, "a_Rotation"},
			BufferElement {4, ShaderDataType::Float3, "a_Size"},
			BufferElement {5, ShaderDataType::Float4, "a_Color"},
			BufferElement {6, ShaderDataType::Float2, "a_TilingFactor"},
			BufferElement {7, ShaderDataType::Int, "a_TextureIndex"}
			});
		s_Data.QuadVAO->AddVertexBuffer(s_Data.QuadBatchBuffer, 1);

		//批处理数据准备（空间准备）
		s_Data.QuadBatchBufferBase = new Quad[Renderer2D_Data::MaxQuads];
		s_Data.QuadBatchBufferPtr = s_Data.QuadBatchBufferBase;

		//Shader
		s_Data.QuadShader = Shader::Create("O:/CppProgram/Ayin/assets/shader/2D/QuadShader.glsl");

		//默认Texture
		s_Data.WhiteTexture = Texture2D::Create(100, 100);

		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		int32_t samplers[32];
		for (int i = 0; i < Renderer2D_Data::MaxTextureSlots; i++) {
			samplers[i] = i;
		}
		s_Data.QuadShader->SetIntArray("u_Textures", samplers, Renderer2D_Data::MaxTextureSlots);

		//? 如果你之前将sampler2D简单视为 int 的话，一定会对 samplers 和u _Texture[32]感到困惑
		//! 不透明类型：在 GLSL 中， sampler2D 属于不透明类型。它既不是指针，也不是简单的整数，而是 GPU 用于访问纹理单元的句柄。
		//! 非法类型转换：你正尝试将整数（ v_TextureIndex ）直接“构造”或“强制转换”为 sampler2D 。虽然这种语法与 C++ 相似，但在 GLSL 中，仅在启用无绑定纹理（Bindless Textures，通过 GL_ARB_bindless_texture 等扩展）的情况下，该语法才合法。
		//  如果不使用samplers和u_Texture[32] 
		//! 由于未启用 bindless texture 扩展，驱动在尝试链接时，发现无法在运行时将随机顶点属性转换为纹理句柄，随即触发了空指针解引用，最终导致程序崩溃。
	};

	void Renderer2D::Shutdown() {
        
        AYIN_PROFILE_FUNCTION();
        
		delete[] s_Data.QuadBatchBufferBase;

	};

	void Renderer2D::BeginScene(const Camera& camera) {

		AYIN_PROFILE_FUNCTION();
	
		s_Data.QuadShader->SetMat4("u_ProjectionViewMatrix", camera.GetProjecttionViewMatrix());


		s_Data.QuadCount = 0;
		s_Data.TextureSlotInsertIndex = 1;
		s_Data.QuadBatchBufferPtr = s_Data.QuadBatchBufferBase;

	};
	void Renderer2D::EndScene() {
		
        AYIN_PROFILE_FUNCTION();

		size_t dataSize = (s_Data.QuadBatchBufferPtr - s_Data.QuadBatchBufferBase) * sizeof(Quad);
		// 指针相减，得到的是基于指针类型的偏移量，并不是字节
		s_Data.QuadBatchBuffer->SetData(s_Data.QuadBatchBufferBase, dataSize);
		
		Flush();

	};


	void Renderer2D::Flush() {

        AYIN_PROFILE_FUNCTION();

		if (s_Data.QuadCount == 0)
			return;

		for (int index = 0; index < s_Data.TextureSlotInsertIndex; index++) {
			s_Data.TextureSlots[index]->Bind(index);
		}

		s_Data.QuadShader->Bind();

		RenderCommand::Draw2DIndexed(s_Data.QuadVAO, s_Data.QuadCount);

		s_Data.QuadShader->UnBind();


		s_Data.Stats.DrawCalls++;
	}


	void Renderer2D::FlushAndReset() {

		EndScene();

		s_Data.QuadCount = 0;
		s_Data.TextureSlotInsertIndex = 1;
		s_Data.QuadBatchBufferPtr = s_Data.QuadBatchBufferBase;

	}
	

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& size, const glm::vec4& color) {
	
        AYIN_PROFILE_FUNCTION();

		if (s_Data.QuadCount >= s_Data.MaxQuads)
			FlushAndReset();

		//! 必须全部设置一遍，因为我们结束渲染时只是简单的移动写指针QuadBatchBufferPtr的位置，没有重置数据
		//! 所以内存中是遗留着旧数据的，如果我们在设置时设置全部数据那就有可能使用到旧的值
		// 数据设置
		s_Data.QuadBatchBufferPtr->Position = position;
		s_Data.QuadBatchBufferPtr->Rotation = rotation;
		s_Data.QuadBatchBufferPtr->Size = size;
		s_Data.QuadBatchBufferPtr->Color = color;
		s_Data.QuadBatchBufferPtr->TilingFactor = glm::vec3{1.0f, 1.0f, 1.0f};
		s_Data.QuadBatchBufferPtr->TextureIndex = 0;


		// s_Data数据设置（写指针，计数，统计计数）
		s_Data.QuadBatchBufferPtr++;
		s_Data.QuadCount++;

		s_Data.Stats.QuadCount++;

	};

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor) {
		
        AYIN_PROFILE_FUNCTION();

		if (s_Data.QuadCount >= s_Data.MaxQuads)
			FlushAndReset();

		// 检查TextureSlots中是否有要用到的纹理，并获取索引
		int textureIndex = 0;
		for (int index = 0; index < s_Data.TextureSlotInsertIndex; index++) {

			if (*(s_Data.TextureSlots[index].get()) == *(texture.get())) {
				textureIndex = index;
				break;
			}

		}
		// TextureSlots中没有就插入纹理
		if (textureIndex == 0) {

			if (s_Data.TextureSlotInsertIndex >= s_Data.MaxTextureSlots)
				FlushAndReset();

			texture->Bind(s_Data.TextureSlotInsertIndex);
			s_Data.TextureSlots[s_Data.TextureSlotInsertIndex] = texture;
			textureIndex = s_Data.TextureSlotInsertIndex++;
		}

		// 数据设置
		s_Data.QuadBatchBufferPtr->Position = position;
		s_Data.QuadBatchBufferPtr->Rotation = rotation;
		s_Data.QuadBatchBufferPtr->Size = size;
		s_Data.QuadBatchBufferPtr->Color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
		s_Data.QuadBatchBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadBatchBufferPtr->TextureIndex = textureIndex;

		// s_Data数据设置（写指针，计数，统计计数）
		s_Data.QuadBatchBufferPtr++;
		s_Data.QuadCount++;

		s_Data.Stats.QuadCount++;


	};


	void Renderer2D::ResetStatistics() {
	
		memset(&s_Data.Stats, 0, sizeof(Renderer2D::Statistics));

	};
	Renderer2D::Statistics Renderer2D::GetStatistics() {
		
		return s_Data.Stats;
	
	};


}

