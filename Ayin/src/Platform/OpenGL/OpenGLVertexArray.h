#pragma once

#include "Ayin/Renderer/VertexArray.h"

namespace Ayin {

	class OpenGLVertexArray : public VertexArray {

	public:

		OpenGLVertexArray();

		~OpenGLVertexArray();

		// 通过 VertexArray 继承
		void Bind() const override;

		void UnBind() const override;

		void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;

		void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

		const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; };

		const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; };


	private:
		uint32_t m_VertexArrayID;

		// 采取引用指针，顶点缓冲与索引缓冲我们认定为一种资源

		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
	};

}