#pragma once

#include <GLM/glm.hpp>

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
		//virtual void Set(const std::string& paramName, void* data) override;
		virtual void Set(void* data) override;

		virtual void SetBindingIndexs(const std::initializer_list<int>& bindingIndexs);

		virtual void Bind() const override;
		virtual void UnBind() const override;

		virtual inline void SetLayout(const UniformLayout& layout) override { m_UniformLayout = layout; };
		virtual inline const UniformLayout& GetLayout() const override { return m_UniformLayout; };


		static inline const std::vector<int>& GetIndexs() { return s_BindingIndexs; };

		virtual inline operator uint32_t() const { return m_UniformBufferID; }

	public:
		//! 关于为什么撤掉了UBO的Set(name,data)
		//! 我不认为那是很好的接口形式，我也没想好该则么样
		//! 同样还有其它API可能参数需求不确定的缘故，但上一个缘故占大头，Shader和OpenGLShader也是如此
		//! 虽然我没想好高层的抽象中该设置一个什么接口，但是底层的情况反而没太多担忧，实现他们是容易的
		void UploadBool(const std::string& name, bool value);


		void UploadInt(const std::string& name, int value);
		void UploadInt2(const std::string& name, const glm::ivec2& vector);
		void UploadInt3(const std::string& name, const glm::ivec3& vector);
		void UploadInt4(const std::string& name, const glm::ivec4& vector);


		void UploadFloat(const std::string& name, float value);
		void UploadFloat2(const std::string& name, const glm::vec2& vector);
		void UploadFloat3(const std::string& name, const glm::vec3& vector);
		void UploadFloat4(const std::string& name, const glm::vec4& vector);


		void UploadMat3(const std::string& name, const glm::mat3& matrix);
		void UploadMat4(const std::string& name, const glm::mat4& matrix);

	private:

		uint32_t m_UniformBufferID;
		UniformLayout m_UniformLayout;

	private:
		static std::vector<int> s_BindingIndexs;

	};


}