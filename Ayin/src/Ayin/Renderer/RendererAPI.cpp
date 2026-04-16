#include <AyinPch.h>

#include "Ayin/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Ayin {

	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

	Scope<RendererAPI> RendererAPI::Create() {
	
		switch (s_API)
		{
			case RendererAPI::API::None:    AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateScope<OpenGLRendererAPI>();
		}

		AYIN_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;

	};


}