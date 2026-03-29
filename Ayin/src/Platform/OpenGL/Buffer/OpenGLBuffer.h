#pragma once

#include "Ayin/Renderer/Buffer.h"


//! 因为OpenGL的状态模式，对于OpenGL相关API的实现，需要注意
//! 只有Bind()以及其它具备状态设置含义的方法（如VAO的SetVBO）可以修改全局状态；
//! 其余API不可以对现有全局状态中有意义的部分造成影响（你可以进行无影响的操作）

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



	//////////////////////////////////////////////////////////////////////////////////////////
	/// UniformBuffer ////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	class AYIN_API OpenGLUniformBuffer : public UniformBuffer {

	public:

		OpenGLUniformBuffer(void* uniformData, size_t size);

		~OpenGLUniformBuffer();

		// 通过 UniformBuffer 继承
		virtual void Set(const std::string& paramName, void* data) override;
		virtual void Set(void* data) override;

		virtual void Bind() const override;
		virtual void UnBind() const override;

		virtual inline void SetLayout(const UniformLayout& layout) override { m_UniformLayout = layout; };
		virtual inline const UniformLayout& GetLayout() const override { return m_UniformLayout; };

		virtual inline void SetIndex(int index) override { m_Index = index; };
		virtual inline int GetIndex() const override { return m_Index; };

	private:

		uint32_t m_UniformBufferID;
		uint32_t m_Index;
		UniformLayout m_UniformLayout;

	};


}