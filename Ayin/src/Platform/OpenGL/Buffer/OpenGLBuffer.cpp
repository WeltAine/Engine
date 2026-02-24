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
	
		int currentIndexBifferID;
		glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &currentIndexBifferID);

		if (currentIndexBifferID == m_IndexBufferID)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	}

}