#pragma once

#include <glm/glm.hpp>
#include "VertexArray.h"

namespace Ayin {

	/// <summary>
	/// 渲染API抽象（就是渲染效果时的指令，比如glElement这些）
	/// 觉得添加一个工厂方法比较合适，然后RenderCommand中设置一个Init方法来配置其s_RendererAPI
	/// </summary>
	class RendererAPI {

	public:
		enum class API {

			None = 0,
			OpenGL = 1

		};


		virtual ~RendererAPI() = default;

		/// <summary>
		/// 初始化，可以设定一些渲染参数（比如混合，深度测试）
		/// </summary>
		virtual void Init() = 0;

		virtual void SetViewport(int x, int y, int width, int height) = 0;

		/// <summary>
		/// 设置清屏时的颜色缓冲
		/// </summary>
		/// <param name="color">颜色</param>
		virtual void SetClearColor(const glm::vec4& color) const = 0;
		/// <summary>
		/// 清屏
		/// </summary>
		virtual void Clear() const = 0;


		/// <summary>
		/// DrawCall
		/// </summary>
		/// <param name="vertexArray">被渲染的VAO</param>
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) const = 0;


		/// <summary>
		/// 获取当前的渲染API
		/// </summary>
		/// <returns></returns>
		inline static API GetAPI() { return s_API; };


	private:
		static API s_API;
	};


}