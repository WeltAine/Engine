#include <AyinPch.h>

#include "Renderer.h"
#include "VertexArray.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Ayin{

	VertexArray* VertexArray::Create() {


		switch (Renderer::GetAPI()) {

			// 这里之后需要用宏来控制，防止去链接设备不支持的代码
			case(RendererAPI::API::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case(RendererAPI::API::OpenGL):	return new OpenGLVertexArray();

		}

		AYIN_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;


	}

}
