#include <AyinPch.h>

#include "Renderer.h"
#include "Buffer.h"
#include "Platform/OpenGL/Buffer/OpenGLBuffer.h"

namespace Ayin {

	VertexBuffer* VertexBuffer::Create(float* vertices, size_t size) {

		switch (Renderer::GetRendererAPI()) {

			// 这里之后需要用宏来控制，防止去链接设备不支持的代码
			case(RendererAPI::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;	
			case(RendererAPI::OpenGL):	return new OpenGLVertexBuffer(vertices, size);

		}

		AYIN_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
		
	}



	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count) {

		switch (Renderer::GetRendererAPI()) {

			// 这里之后需要用宏来控制，防止去链接设备不支持的代码
			case(RendererAPI::None):	AYIN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case(RendererAPI::OpenGL):	return new OpenGLIndexBuffer(indices, count);

		}

		AYIN_CORE_ASSERT(false, "Unknown RendererAPI!");

	}

}