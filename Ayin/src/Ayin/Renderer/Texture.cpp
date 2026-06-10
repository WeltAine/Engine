#include <AyinPch.h>

#include "Ayin/Renderer/Texture.h"
#include "Ayin/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Ayin {

	std::unordered_map<std::string, Ref<Texture2D>> Texture2DLibrary::s_Textures;


	Ref<Texture2D> Texture2D::Create(const std::string& path) {

		switch (Renderer::GetAPI()) {

			// 这里之后需要用宏来控制，防止去链接设备不支持的代码
			case(RendererAPI::API::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case(RendererAPI::API::OpenGL):	return std::make_shared<OpenGLTexture2D>(path);

		}

		AYIN_CORE_ASSERT(false, "Unknown RendererAPI!");


	}

	Ref<Texture2D> Texture2D::Create(int width, int height, void* data) {

		switch (Renderer::GetAPI()) {

			// 这里之后需要用宏来控制，防止去链接设备不支持的代码
			case(RendererAPI::API::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case(RendererAPI::API::OpenGL):	return std::make_shared<OpenGLTexture2D>(width, height, data);

		}

		AYIN_CORE_ASSERT(false, "Unknown RendererAPI!");


	}


	Ref<Texture2D> Texture2DLibrary::Load(const std::string& filePath) {

		// 查找名称
		size_t lastSlash = filePath.find_last_of("/\\");//找"/"或者"\"
		//! 该方法的含义为，查找字符串中最后出现过参数字符串中任意一个字符的位置（就像拿着一系列嫌疑人画像（参数），在监控里（对象）查找最后一个出现的嫌疑人）
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;

		size_t lastDot = filePath.rfind('.');//查找最后出现的目标（完全匹配）
		lastDot = lastDot == std::string::npos ? filePath.size() : lastDot;

		size_t count = lastDot - lastSlash;

		std::string name = filePath.substr(lastSlash, count);

		if(!Exists(name))
			Add(name, Texture2D::Create(filePath));

		return s_Textures[name];

	};

	void Texture2DLibrary::Add(const std::string& name, const Ref<Texture2D>& texture) {

		AYIN_CORE_ASSERT(!Exists(name), "That name already exists in the Texture2DLibrary");
		s_Textures[name] = texture;

	};


	Ref<Texture2D> Texture2DLibrary::Get(const std::string& name) {

		if (Exists(name))
			return s_Textures[name];

		return nullptr;

	};


	bool Texture2DLibrary::Exists(const std::string& name) {

		return s_Textures.find(name) != s_Textures.end();

	};


}
