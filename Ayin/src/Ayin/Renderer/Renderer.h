#pragma once

#include "RenderCommand.h"

// 渲染器Renderer的部分实现
namespace Ayin {

	class AYIN_API Renderer 
	{
	public:

		/// <summary>
		/// 场景/环境设置（相机位置，光照。。。）
		/// </summary>
		inline static void BeginScene() {};//这些场景数据会作为后续shader的统一变量

		inline static void EndScene() {};


		inline static void Submit(const std::shared_ptr<VertexArray>& vertexArray) {
			RenderCommand::DrawIndexed(vertexArray);
		}

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); };

	};
}