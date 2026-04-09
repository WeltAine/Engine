#include <AyinPch.h>

#include "Renderer.h"
#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Ayin {

	Scope<RendererAPI> RenderCommand::s_RendererAPI = nullptr;

	void RenderCommand::Init() {

		switch (Renderer::GetAPI()) {

			// 这里之后需要用宏来控制，防止去链接设备不支持的代码
			case(RendererAPI::API::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			case(RendererAPI::API::OpenGL):	s_RendererAPI.reset(new OpenGLRendererAPI); break;

		}

		s_RendererAPI->Init();

	}

}