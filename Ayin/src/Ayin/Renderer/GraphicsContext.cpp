#include <AyinPch.h>

#include "Ayin/Renderer/Renderer.h"
#include "Ayin/Renderer/GraphicsContext.h"

#include "Platform/OpenGL/OpenGLContext.h"

namespace Ayin {

	Scope<GraphicsContext> GraphicsContext::Create(Window* window)
	{

		switch (Renderer::GetAPI()) {
			case(RendererAPI::API::None) : AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case(RendererAPI::API::OpenGL): return std::make_unique<OpenGLContext>(window);
		}

		AYIN_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}


}