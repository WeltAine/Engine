#include <AyinPch.h>

#include "Ayin/Renderer/Renderer.h"
#include "Ayin/Renderer/Buffer.h"

#include "Platform/OpenGL/Buffer/OpenGLBuffer.h"

namespace Ayin {

	//////////////////////////////////////////////////////////////////////////////////////////
	/// VertexBuffer /////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////


	Ref<VertexBuffer> VertexBuffer::Create(size_t size) {

		switch(Renderer::GetAPI()) {

			// 这里之后需要用宏来控制，防止去链接设备不支持的代码
			case(RendererAPI::API::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case(RendererAPI::API::OpenGL):	return std::make_shared<OpenGLVertexBuffer>(size);

		}

		AYIN_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;

	}



	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, size_t size) {

		switch (Renderer::GetAPI()) {

			// 这里之后需要用宏来控制，防止去链接设备不支持的代码
			case(RendererAPI::API::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;	
			case(RendererAPI::API::OpenGL):	return std::make_shared<OpenGLVertexBuffer>(vertices, size);

		}

		AYIN_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
		
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	/// IndexBuffer //////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count) {

		switch (Renderer::GetAPI()) {

			// 这里之后需要用宏来控制，防止去链接设备不支持的代码
			case(RendererAPI::API::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case(RendererAPI::API::OpenGL):	return std::make_shared<OpenGLIndexBuffer>(indices, count);

		}

		AYIN_CORE_ASSERT(false, "Unknown RendererAPI!");

	}


	//////////////////////////////////////////////////////////////////////////////////////////
	/// UniformBuffer ////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////
	Ref<UniformBuffer> UniformBuffer::Create(void* data, size_t size) {

		switch (Renderer::GetAPI()) {

			case(RendererAPI::API::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case(RendererAPI::API::OpenGL): return std::make_shared<OpenGLUniformBuffer>(data, size);

		}

	}

	const std::vector<int>& UniformBuffer::GetIndexs() {

		switch (Renderer::GetAPI()) {

		case(RendererAPI::API::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return std::vector<int>();
			case(RendererAPI::API::OpenGL): return OpenGLUniformBuffer::GetIndexs();

		}

	}


}