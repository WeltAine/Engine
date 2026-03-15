#include <AyinPch.h>
#include "OpenGLBuffer.h"

#include <Glad/glad.h>

namespace Ayin {

	//////////////////////////////////////////////////////////////////////////////////////////
	/// VertexBuffer /////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, size_t size) {

		glCreateBuffers(1, &m_VertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer() {

		glDeleteBuffers(1, &m_VertexBufferID);
		UnBind();

	}

	void OpenGLVertexBuffer::Bind() const { 

		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID); 

	}

	void OpenGLVertexBuffer::UnBind() const {

		int currentVertexBufferID = 0;
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVertexBufferID);

		if (currentVertexBufferID == m_VertexBufferID)
			glBindBuffer(GL_ARRAY_BUFFER, 0);

	}



	//////////////////////////////////////////////////////////////////////////////////////////
	/// IndexBuffer //////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count) 
		: m_Count(count)
	{
		glCreateBuffers(1, &m_IndexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer() {

		glDeleteBuffers(1, &m_IndexBufferID);
		UnBind();

	}

	void OpenGLIndexBuffer::Bind() const { 

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferID); 

	}

	void OpenGLIndexBuffer::UnBind() const {
	
		int currentIndexBufferID;
		glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &currentIndexBufferID);

		if (currentIndexBufferID == m_IndexBufferID)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	}


	//////////////////////////////////////////////////////////////////////////////////////////
	/// UniformBuffer ////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	OpenGLUniformBuffer::OpenGLUniformBuffer(void* uniformData, size_t size)
	{

		//glBufferData(目标，大小，数据，用法);
		//glBindBufferBase(目标，索引（bind）, 对象);	//也可以不指定（注意不会设置当前活跃的UBO，只是指定，shader中没有这个block块也没关系）
		//glBufferSubData(目标, 起始偏移, 大小，数据);//向当前活跃对象上传数据，偏移是GPU中block内的偏移
		//glBindBufferRange(目标，索引（bind），对象，起始偏移，总大小);//划分对象数据段到指定bind


		int currentUniformBufferID = 0;
		glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &currentUniformBufferID);

		// 分配总显存大小
		glCreateBuffers(1, &m_UniformBufferID);
		glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferID);
		glBufferData(GL_UNIFORM_BUFFER, size, uniformData, GL_DYNAMIC_DRAW);	// 之后移动走，不由Uniform、开辟，由Shader统一开辟


		glBindBuffer(GL_UNIFORM_BUFFER, currentUniformBufferID);

	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer() {

		UnBind();
		glDeleteBuffers(1, &m_UniformBufferID);

	}


	void OpenGLUniformBuffer::Set(const std::string& paramName, void* data)
	{
		auto it = std::find_if(m_UniformLayout.begin(), m_UniformLayout.end(), [&paramName](const UniformElement& element) {
			return element.Name == paramName;
			});

		AYIN_ASSERT(it != m_UniformLayout.end(), "Unfind {0} in shader!", paramName);

		Bind();

		// 设置UBO对应数据块上某一段区域数据
		glBufferSubData(GL_UNIFORM_BUFFER, it->Offset, it->Size, data);

		UnBind();

	}

	void OpenGLUniformBuffer::Set(void* data)
	{
		Bind();

		// 设置UBO对应数据块整个区域数据
		glBufferSubData(GL_UNIFORM_BUFFER, 0, m_UniformLayout.GetSize(), data);

		UnBind();
	}


	void OpenGLUniformBuffer::Bind() const
	{
		glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferID);					// 设定当前活跃UBO
		glBindBufferBase(GL_UNIFORM_BUFFER, m_Index, m_UniformBufferID);	// UBO绑定到对应binding上
	}


	void OpenGLUniformBuffer::UnBind() const
	{
		int currentUniformBufferID = 0;
		int currentIndexUniformBufferID = 0;
		glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &currentUniformBufferID);
		glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, m_Index, &currentIndexUniformBufferID);

		if (currentUniformBufferID == m_UniformBufferID)
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

		if (currentIndexUniformBufferID == m_UniformBufferID)
			glBindBufferBase(GL_UNIFORM_BUFFER, m_Index, 0);

	}


}