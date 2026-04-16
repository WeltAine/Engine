#include "AyinPch.h"

#include "Platform/OpenGL/OpenGLVertexArray.h"

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

	#pragma region BTA/DSA
		glCreateVertexArrays(1, &m_VertexArrayID);
	#pragma endregion

	}

	OpenGLVertexArray::~OpenGLVertexArray() {

	#pragma region BTA/DSA
		UnBind();
		glDeleteVertexArrays(1, &m_VertexArrayID);
	#pragma endregion

	}


	void OpenGLVertexArray::Bind() const
	{

	#pragma region BTA/DSA
		glBindVertexArray(m_VertexArrayID);
	#pragma endregion 

	}


	void OpenGLVertexArray::UnBind() const
	{

	#pragma region BTA/DSA
		int currentVertexArrayID = 0;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVertexArrayID);

		if (currentVertexArrayID == m_VertexArrayID)
			glBindVertexArray(0);
	#pragma endregion

	}

	//? 有严重缺陷，如果vertexbuffer分开给的话，原先的方法会导致index冲突，我们先在BufferElement中添加一个locationIndex
	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		AYIN_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

	#pragma region BTA（该注释中具备一定教育意义，先留着，下次完善时再删除）
		//// 收集上下文环境（用于恢复）
		//int lastVertexArrayID = 0, lastVertexBufferID = 0;
		//glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVertexArrayID);
		//glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lastVertexBufferID);

		//// 配置顶点
		//glBindVertexArray(m_VertexArrayID);
		//vertexBuffer->Bind();

		//for (const auto& element : vertexBuffer->GetLayout()) {

		//	glVertexAttribPointer(
		//		element.LocationIndex,									// location
		//		element.GetComponentCount(),							// 基本类型的数量
		//		ShaderDataTypeToBaseType(element.Type),					// 基本类型
		//		element.Normalized,										// 是否归一化
		//		vertexBuffer->GetLayout().GetStride(),					// 步进
		//		(const void*)(long long int)element.Offset				// GPU块中的起始偏移
		//		//! 在原类型大小小于系统的指针长度时不要直接转void
		//		//! 在有存储相关优化的情况下32->64可能会给高32位填垃圾值
		//		//! 所以我们先转long long int
		//	);
		//	glEnableVertexAttribArray(element.LocationIndex);
		//}

		//m_VertexBuffers.push_back(vertexBuffer);

		////恢复上下文环境
		//glBindVertexArray(lastVertexArrayID);
		//glBindBuffer(GL_ARRAY_BUFFER, lastVertexBufferID);
	#pragma endregion


	#pragma region DSA

		int maxVAOBindingIndexs = 0, maxVAOAttribIndexs = 0;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &maxVAOBindingIndexs);
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVAOAttribIndexs);

		//将缓冲链接到BindingIndex上，并设置缓冲读指针移动方式（每一次移动就意味着一次吐出行为，每一次吐出都吐出一行（条）数据项）
		glVertexArrayVertexBuffer(
			m_VertexArrayID,								//目标VAO
			m_BindingIndex,									//VBO占用目标VAO的哪一个BindingIndex
			*vertexBuffer,									//VBO
			(long long int)0,								//指针起始位置
			vertexBuffer->GetLayout().GetStride()			//指针移动量（一次吐出量）
		);

		//缓冲读指针的移动频率设置（0：每个顶点移动一次，n：多少个实例（模型）移动一次）
		glVertexArrayBindingDivisor(
			m_VertexArrayID,								//目标VAO 
			m_BindingIndex,									//要设置VAO的哪一个BindingIndex的数据吐出频率（）直接作用在绑定于该BindingIndex的缓冲读指针，决定了指针何时移动（可用于批量渲染优化）
			0												//设置吐出频率（实例的意思可以理解为一个模型）
		);

		for (const auto& element : vertexBuffer->GetLayout()) {

			AYIN_CORE_ASSERT((element.LocationIndex < maxVAOBindingIndexs), "Exceeds the maximum VAOBindingIndex");

			glEnableVertexArrayAttrib(m_VertexArrayID, element.LocationIndex);

			//连接AttribIndex和BindingIndex
			glVertexArrayAttribBinding(
				m_VertexArrayID,							//目标VAO
				element.LocationIndex,						//目标VAO的哪一个AttribIndex要进行连接配置
				m_BindingIndex								//连接到哪一个BindingIndex以对缓冲进行数据访问
			);


			//AttribIndex读取设置（从bindingIndex中突出的数据中如何读取）
			glVertexArrayAttribFormat(
				m_VertexArrayID,							//目标VAO
				element.LocationIndex,						//设置目标VAO的哪一个AttribIndex
				element.GetComponentCount(),				//从缓冲吐出中的读取量（配合后一个参数）
				ShaderDataTypeToBaseType(element.Type),		//从缓冲吐出中的读取量（配合前一个参数）
				element.Normalized,							//是否归一化
				element.Offset								//缓冲吐出块中的读取偏移
			);

		}

		m_VertexBuffers.push_back(vertexBuffer);

		AYIN_CORE_ASSERT((m_BindingIndex + 1 < maxVAOAttribIndexs), "Exceeds the maximum VAOAttribIndex");
		m_BindingIndex = (m_BindingIndex + 1) % maxVAOAttribIndexs;
	#pragma endregion

	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
	#pragma region BTA
		//// 收集上下文环境（用于恢复）
		//int lastVertexArrayID = 0, lastIndexBufferID = 0;
		//glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVertexArrayID);
		//glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &lastIndexBufferID);


		//glBindVertexArray(m_VertexArrayID);
		//indexBuffer->Bind();

		//m_IndexBuffer = indexBuffer;
		//
		//// 恢复上下文环境
		//glBindVertexArray(lastVertexArrayID);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lastIndexBufferID);
	#pragma endregion

	#pragma region DSA
		glVertexArrayElementBuffer(m_VertexArrayID, *indexBuffer);
		m_IndexBuffer = indexBuffer;
	#pragma endregion
	}

}

