#pragma once

#include "Ayin/Core.h"

#include "RendererAPI.h"

namespace Ayin {

	/// <summary>
	/// 渲染命令，会将一系列指令分发给RendererAPI
	/// 外观模式
	/// 可以认为是对RendererAPI的静态封装，这么设计的原因我还不是很明白，可能是从“语义”上来说，并不期望其存在实例，仅仅只是方法的集合，另一种一种原因或许是受Renderer影响（但我觉得不像），晚点问问
	/// </summary>
	class AYIN_API RenderCommand {

	public:
		// 工厂方法（我觉得这样更好就加了，在init中决定RendererAPI）
		//x 没想到我竟然提前就干了这种事情
		static void Init();

		inline static void SetViewport(int x, int y, int width, int height) { s_RendererAPI->SetViewport(x, y, width, height); };

		inline static void SetClearColor(const glm::vec4& color) { s_RendererAPI->SetClearColor(color); };

		inline static void Clear() { s_RendererAPI->Clear(); };

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray) {
			vertexArray->Bind();
			s_RendererAPI->DrawIndexed(vertexArray); 
		};


	private:
		static Scope<RendererAPI> s_RendererAPI;

	};

}