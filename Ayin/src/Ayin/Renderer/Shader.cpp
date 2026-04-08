#include <AyinPch.h>

#include "Shader.h"
#include "Ayin/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

// 目前直接作为OpenGL侧的实现（之后会拆出，如果有时间接入其它API）
namespace Ayin {

	//////////////////////////////////////////////////////////////////////////////////////////
	/// Shader ///////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////


	Ref<Shader> Shader::Create(const std::string& filePath) {

		//? 有一说一，ReadFile是不是放在上层比较好，而不是放到实现侧（Cherno倒是说过相关的内容，虚拟文件系统之类的），目前来说如何读取还不重要

		switch (Renderer::GetAPI()) {

			case(RendererAPI::API::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case(RendererAPI::API::OpenGL): return std::make_shared<OpenGLShader>(filePath);

		}
	}

	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexShaderSrc, const std::string& fragmentShaderSrc) {

		switch (Renderer::GetAPI()) {

			case(RendererAPI::API::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case(RendererAPI::API::OpenGL): return std::make_shared<OpenGLShader>(name, vertexShaderSrc, fragmentShaderSrc);

		}

	}



	//////////////////////////////////////////////////////////////////////////////////////////
	/// ShaderLibrary ////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filePath) {

		Ref<Shader> shader = Shader::Create(filePath);
		Add(name, shader);

		return shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& filePath) {

		Ref<Shader> shader = Shader::Create(filePath);
		Add(shader);

		return shader;
	}


	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader) {

		AYIN_CORE_ASSERT(!Exists(name), "That name already exists in the ShaderLibrary");
		m_Shaders[name] = shader;

	}

	void ShaderLibrary::Add(const Ref<Shader>& shader) {
		Add(shader->GetName(), shader);
	}

	bool ShaderLibrary::Exists(const std::string& name) const {
		return m_Shaders.find(name) != m_Shaders.end();
	}

}