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

		virtual inline operator uint32_t() const { return m_VertexBufferID; };


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

		virtual inline operator uint32_t() const { return m_IndexBufferID; };

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

		virtual void SetBindingIndexs(const std::initializer_list<int>& bindingIndexs);

		virtual void Bind() const override;
		virtual void Bind(int blockIndex) override;
		virtual void UnBind() const override;

		virtual inline void SetLayout(const UniformLayout& layout) override { m_UniformLayout = layout; };
		virtual inline const UniformLayout& GetLayout() const override { return m_UniformLayout; };


		virtual inline const std::vector<int>& GetIndex() const override { return m_BindingIndexs; };

		virtual inline operator uint32_t() const { return m_UniformBufferID; }

	private:

		uint32_t m_UniformBufferID;
		UniformLayout m_UniformLayout;
		std::vector<int> m_BindingIndexs;

	};


}