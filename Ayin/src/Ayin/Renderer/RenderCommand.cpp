#include <AyinPch.h>

#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Ayin {

	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;// 虽然之前一直没注意，这不会导致内存泄露么

}