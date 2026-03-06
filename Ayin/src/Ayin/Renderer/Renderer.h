#pragma once

#include "RenderCommand.h"

#include "Camera.h"
#include "Shader.h"

// 渲染器Renderer的部分实现
namespace Ayin {

	class AYIN_API Renderer 
	{
	public:

		/// <summary>
		/// 场景/环境设置（相机位置，光照。。。）
		/// </summary>
		static void BeginScene(const Camera& rendererCamera);//这些场景数据会作为后续shader的统一变量

		static void EndScene();


		static void Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray);//之后这些参数会被并入网格和材质中，网格中包含材质

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); };

	private:

		// 因为shader需要vp数据，而渲染的vp数据隶属于渲染相机，属于“环境”信息的一部分
		// 同时Renderer类是一个静态类，环境数据不适合再提交时才传入，而应该再BeginScene中传入
		// 所以需要设计一个静态成员来存放场景数据
		struct SceneData {

			glm::mat4 projectionViewMatrix;

		};

		static SceneData* s_SceneData;
	};
}