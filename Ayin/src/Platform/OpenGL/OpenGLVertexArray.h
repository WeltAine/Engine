#pragma once

#include "Ayin/Renderer/VertexArray.h"

//! 因为OpenGL的状态模式，对于OpenGL相关API的实现，需要注意
//! 只有Bind()以及其它具备状态设置含义的方法（如VAO的SetVBO）可以修改全局状态；
//! 其余API不可以对现有全局状态中有意义的部分造成影响（你可以进行无影响的操作）


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
	};

}