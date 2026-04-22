#pragma once


#include "Ayin/Core/Core.h"
#include "Ayin/Renderer/VertexArray.h"
#include "Ayin/Renderer/Shader.h"
#include "Ayin/Renderer/Camera.h"
#include "Ayin/Renderer/Texture.h"

#include <glm/glm.hpp>


namespace Ayin {


	struct Quad {

		glm::vec3 Position;
		glm::vec3 Rotation;
		glm::vec3 Size;
		glm::vec4 Color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};

		glm::vec2 TilingFactor = glm::vec2{1.0f, 1.0f};
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

		static const  uint32_t MaxQuads = 10000;
		static constexpr uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> m_QuadVAO;			//默认矩形
		Ref<VertexBuffer> m_QuadBuffer;		//矩形数据（模型频率）
		Quad* m_QuadBufferBase = nullptr;
		Quad* m_QuadBufferPtr = nullptr;

		uint32_t m_QuadCount = 0;

		Ref<Shader> m_QuadShader;			//默认Shader

		Ref<Texture2D> m_WhiteTexture;		//默认纹理

		std::array<Ref<Texture2D>, MaxTextureSlots> m_TextureSlots;	//纹理单元与其纹理管理
		uint32_t m_TextureSlotInsertIndex = 1;						//纹理可插入位置

	};

	AYIN_API class Renderer2D {

	public:

		static void Init();

		static void Shutdown();

		static void BeginScene(const Camera& camera);
		static void EndScene();
		
		static void Flush();

		static void DrawQuad(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& size, const glm::vec4& color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
		static void DrawQuad(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2{1.0f, 1.0f});

	private:

		static Renderer2D_Data s_Data;

	};

}