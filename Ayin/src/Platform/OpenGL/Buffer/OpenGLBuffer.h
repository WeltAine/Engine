#pragma once

#include "Ayin/Renderer/Buffer.h"

namespace Ayin {

	//////////////////////////////////////////////////////////////////////////////////////////
	/// VertexBuffer /////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////


	class OpenGLVertexBuffer : public VertexBuffer 
	{

	public:

		OpenGLVertexBuffer(float* vertices, size_t size);

		// 通过 VertexBuffer 继承
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;

		virtual void UnBind() const override;

		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; };

		virtual const BufferLayout& GetLayout() const { return m_Layout; };

	private:
		uint32_t m_VertexBufferID;
		BufferLayout m_Layout;
	};



	//////////////////////////////////////////////////////////////////////////////////////////
	/// IndexBuffer //////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	class OpenGLIndexBuffer : public IndexBuffer
	{

	public:

		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);

		// 通过 IndexBuffer 继承
		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const override;

		virtual void UnBind() const override;

		virtual uint32_t GetCount() const override { return m_Count; };

	private:
		uint32_t m_IndexBufferID;
		uint32_t m_Count;
	};


}