#include <AyinPch.h>

#include "Renderer.h"
#include "Buffer.h"
#include "Platform/OpenGL/Buffer/OpenGLBuffer.h"

namespace Ayin {

	//////////////////////////////////////////////////////////////////////////////////////////
	/// VertexBuffer /////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	VertexBuffer* VertexBuffer::Create(float* vertices, size_t size) {

		switch (Renderer::GetAPI()) {

			// 这里之后需要用宏来控制，防止去链接设备不支持的代码
			case(RendererAPI::API::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;	
			case(RendererAPI::API::OpenGL):	return new OpenGLVertexBuffer(vertices, size);

		}

		AYIN_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
		
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	/// IndexBuffer //////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count) {

		switch (Renderer::GetAPI()) {

			// 这里之后需要用宏来控制，防止去链接设备不支持的代码
			case(RendererAPI::API::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case(RendererAPI::API::OpenGL):	return new OpenGLIndexBuffer(indices, count);

		}

		AYIN_CORE_ASSERT(false, "Unknown RendererAPI!");

	}


	//////////////////////////////////////////////////////////////////////////////////////////
	/// UniformBuffer ////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////
	UniformBuffer* UniformBuffer::Create(void* data, size_t size) {

		switch (Renderer::GetAPI()) {

		case(RendererAPI::API::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case(RendererAPI::API::OpenGL): return new OpenGLUniformBuffer(data, size);

		}

	}


}