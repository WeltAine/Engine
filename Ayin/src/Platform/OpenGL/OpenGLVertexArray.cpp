#include "AyinPch.h"
#include "OpenGLVertexArray.h"

#include <Glad/glad.h>


namespace Ayin {


	//之后会移除（先应急一下，做个测试）
	static GLenum ShaderDataTypeToBaseType(ShaderDataType type) {

		switch (type) {

		case ShaderDataType::Float:    return GL_FLOAT;
		case ShaderDataType::Float2:   return GL_FLOAT;
		case ShaderDataType::Float3:   return GL_FLOAT;
		case ShaderDataType::Float4:   return GL_FLOAT;
		case ShaderDataType::Int:      return GL_INT;
		case ShaderDataType::Int2:     return GL_INT;
		case ShaderDataType::Int3:     return GL_INT;
		case ShaderDataType::Int4:     return GL_INT;
		case ShaderDataType::Bool:     return GL_BOOL;
		case ShaderDataType::Mat3:     return GL_FLOAT;
		case ShaderDataType::Mat4:     return GL_FLOAT;

		}

		AYIN_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;

	}


	OpenGLVertexArray::OpenGLVertexArray() {
		glCreateVertexArrays(1, &m_VertexArrayID);
	}

	OpenGLVertexArray::~OpenGLVertexArray() {

		UnBind();

		glDeleteVertexArrays(1, &m_VertexArrayID);
	}


	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_VertexArrayID);
	}


	void OpenGLVertexArray::UnBind() const
	{
		int currentVertexArrayID = 0;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVertexArrayID);

		if (currentVertexArrayID == m_VertexArrayID)
			glBindVertexArray(0);
	}

	//? 有严重缺陷，如果vertexbuffer分开给的话，原先的方法会导致index冲突，我们先在BufferElement中添加一个locationIndex
	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		AYIN_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		// 收集上下文环境（用于恢复）
		int lastVertexArrayID = 0, lastVertexBufferID = 0;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVertexArrayID);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lastVertexBufferID);

		// 配置顶点
		glBindVertexArray(m_VertexArrayID);
		vertexBuffer->Bind();

		int index = 0;
		for (const auto& element : vertexBuffer->GetLayout()) {

			glVertexAttribPointer(
				element.LocationIndex,									// location
				element.GetComponentCount(),							// 基本类型的数量
				ShaderDataTypeToBaseType(element.Type),					// 基本类型
				element.Normalized,										// 是否归一化
				static_cast<int>(vertexBuffer->GetLayout().GetStride()),// 步进
				(const void*)(long long int)element.Offset				// GPU块中的起始偏移
				//! 在原类型大小小于系统的指针长度时不要直接转void
				//! 在有存储相关优化的情况下32->64可能会给高32位填垃圾值
				//! 所以我们先转long long int
			);
			glEnableVertexAttribArray(index);
			index++;
		}

		m_VertexBuffers.push_back(vertexBuffer);

		//恢复上下文环境
		glBindVertexArray(lastVertexArrayID);
		glBindBuffer(GL_ARRAY_BUFFER, lastVertexBufferID);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		// 收集上下文环境（用于恢复）
		int lastVertexArrayID = 0, lastIndexBufferID = 0;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVertexArrayID);
		glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &lastIndexBufferID);


		glBindVertexArray(m_VertexArrayID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
		
		// 恢复上下文环境
		glBindVertexArray(lastVertexArrayID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lastIndexBufferID);
	}

}

