#include <AyinPch.h>

#include "Ayin/Renderer/Renderer.h"
#include "Ayin/Renderer/RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Ayin {

	Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();

}