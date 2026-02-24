#pragma once

#include <AyinPch.h>

// 渲染器Renderer的部分实现（还没到实现它的时候）

namespace Ayin {

	enum class RendererAPI {

		None = 0,
		OpenGL = 1

	};

	class AYIN_API Renderer 
	{
	public:
		inline static RendererAPI GetRendererAPI() { return s_RendererAPI; };

	private:
		static RendererAPI s_RendererAPI;
	};
}