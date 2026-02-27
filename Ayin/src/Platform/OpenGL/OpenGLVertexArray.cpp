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

		int currentVertexArrayID = 0;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVertexArrayID);

		if (currentVertexArrayID == m_VertexArrayID)
			glBindVertexArray(0);

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


	void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
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
				index,
				element.GetComponentCount(),
				ShaderDataTypeToBaseType(element.Type),
				element.Normalized,
				vertexBuffer->GetLayout().GetStride(),
				(const void*)element.Offset
			);
			glEnableVertexAttribArray(index);
			index++;
		}

		m_VertexBuffers.push_back(vertexBuffer);

		//恢复上下文环境
		glBindVertexArray(lastVertexArrayID);
		glBindBuffer(GL_ARRAY_BUFFER, lastVertexBufferID);
	}

	void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
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

