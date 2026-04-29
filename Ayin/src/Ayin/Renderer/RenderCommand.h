#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Renderer/RendererAPI.h"

namespace Ayin {

	/// <summary>
	/// 渲染命令，会将一系列指令分发给RendererAPI
	/// 外观模式
	/// 可以认为是对RendererAPI的静态封装，这么设计的原因我还不是很明白，可能是从“语义”上来说，并不期望其存在实例，仅仅只是方法的集合，另一种一种原因或许是受Renderer影响（但我觉得不像），晚点问问
	/// </summary>
	class AYIN_API RenderCommand {

	public:
		
		inline static void Init() { s_RendererAPI->Init(); };

		inline static void SetViewport(int x, int y, int width, int height) { s_RendererAPI->SetViewport(x, y, width, height); };

		inline static void SetClearColor(const glm::vec4& color) { s_RendererAPI->SetClearColor(color); };

		inline static void Clear() { s_RendererAPI->Clear(); };

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray) {
			vertexArray->Bind();
			s_RendererAPI->DrawIndexed(vertexArray); 
			vertexArray->UnBind();
		};


		inline static void Draw2DIndexed(const Ref<VertexArray>& vertexArray, int count = 0) {

			vertexArray->Bind();
			s_RendererAPI->Draw2DIndexed(vertexArray, count);
			vertexArray->UnBind();

		}

	private:
		static Scope<RendererAPI> s_RendererAPI;

	};

}