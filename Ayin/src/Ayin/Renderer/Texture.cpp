#include <AyinPch.h>

#include "Texture.h"
#include "Ayin/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Ayin {

	
	Ref<Texture2D> Texture2D::Create(const std::string& path) {

		switch (Renderer::GetAPI()) {

			// 这里之后需要用宏来控制，防止去链接设备不支持的代码
			case(RendererAPI::API::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case(RendererAPI::API::OpenGL):	return std::make_shared<OpenGLTexture2D>(path);

		}

		AYIN_CORE_ASSERT(false, "Unknown RendererAPI!");


	}


}