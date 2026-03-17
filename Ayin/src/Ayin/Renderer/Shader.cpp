#include <AyinPch.h>

#include "Shader.h"
#include "Ayin/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

// 目前直接作为OpenGL侧的实现（之后会拆出，如果有时间接入其它API）
namespace Ayin {

	Shader* Shader::Create(const std::string& vertexShaderSrc, const std::string& fragmentShaderSrc) {

		switch (Renderer::GetAPI()) {

		case(RendererAPI::API::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case(RendererAPI::API::OpenGL): return new OpenGLShader(vertexShaderSrc, fragmentShaderSrc);

		}

	}

}