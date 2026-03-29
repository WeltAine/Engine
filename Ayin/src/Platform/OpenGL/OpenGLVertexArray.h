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

		void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;

		void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

		const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; };

		const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; };


	private:
		uint32_t m_VertexArrayID;

		// 采取引用指针，顶点缓冲与索引缓冲我们认定为一种资源

		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;

		uint32_t m_BindingIndex = 0;//DSA下，下一次记录VBO时要使用的BindingIndex

	};

}