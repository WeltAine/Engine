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
	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer, uint32_t divisor)
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

		//! AttribIndex根据配置从一个BindingIndex的吐出中读取数据，通过Fomat传递给Shader程序中的attribute 
		//!		BindingIndex通过配置，按一定频率，从吧绑定的VBO中读取一定数据，吐出给AttribIndex
		//! 					 
		//!						  Index配对             VBO绑定
		//!				AttribIndex			 BindingIndex
		//!				   [0] -------\			 [0] <-----------|
		//!				   [1]		   \		 [1] <---------	VBO
		//!				   [2]			-------- [2] <-----------|
		//!		VAO			*					  *				
		//!				    *					  *
		//!				    *					  *
		//!				   [n] ----------------- [n] <--------- VBO
		//! 
		//!								吐出指针和指针移动频率
		//! 
		//!		  针对吐出的读取指针配置
		//


		////设置a_index和b_index的链接关系
		//glVertexArrayAttribBinding(vao, a_index, b_index);
		
		////设置b_index对应的buffer对象，shader数据上传方式?
		//glVertexArrayVertexBuffer(vao, b_index, buffer, offset, stride);

		////设置b_index中VBO的数据吐出模式
		//glVertexArrayBindingDivisor(vao, b_index, divisor);//不是glVertexBufferBindingDivisor
		
		////设置a_index的读取模式
		//glVertexArrayAttribFormat(vao, a_index, size, type, normalized, offset);


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
			divisor											//设置吐出频率（实例的意思可以理解为一个模型）
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


			if (
				(int)element.Type 
				& 
				[]() constexpr -> int {
					return (int)ShaderDataType::Float
						| (int)ShaderDataType::Float2
						| (int)ShaderDataType::Float3
						| (int)ShaderDataType::Float4;}()
				) 
			{

				//AttribIndex读取设置（从bindingIndex中吐出的数据中如何读取）
				glVertexArrayAttribFormat(
					m_VertexArrayID,							//目标VAO
					element.LocationIndex,						//设置目标VAO的哪一个AttribIndex
					element.GetComponentCount(),				//从一次缓冲吐出中的读取量（配合后一个参数）
					ShaderDataTypeToBaseType(element.Type),		//从一次缓冲吐出中的读取量（配合前一个参数）
					element.Normalized,							//是否归一化()
					element.Offset								//缓冲吐出块中的读取偏移
				);

			}
			else if(
				(int)element.Type
				&
				[]() constexpr -> int {
					return (int)ShaderDataType::Bool
						| (int)ShaderDataType::Int
						| (int)ShaderDataType::Int2
						| (int)ShaderDataType::Int3
						| (int)ShaderDataType::Int4; }()
				) 
			{

				glVertexArrayAttribIFormat(
					m_VertexArrayID,
					element.LocationIndex,
					element.GetComponentCount(),
					ShaderDataTypeToBaseType(element.Type),
					element.Offset
				);

			}
			else if(
				(int)element.Type
				&
				[]() constexpr -> int {
					return (int)ShaderDataType::Mat3
						| (int)ShaderDataType::Mat4; }()
				) 
			{

				//! 从历史发展来看，GPU 计算的基本单位曾是 128 位寄存器。由于 3D 图形是基于齐次坐标 $(x, y, z, w)$ 和颜色值 $(r, g, b, a)$ 构建的。
				//! GPU 厂商针对性地优化了硬件设计，使其能够在一个时钟周期内同时处理四个 32 位浮点数（即一个向量）。
				//! 
				//! SIMD 架构：早期的 GPU 采用专门针对四维向量设计的 SIMD（单指令多数据）单元。
				//! 在物理硬件层面，一个“顶点属性槽”被直接布线为 128 位宽的端口。
				//! 
				//! 在 GLSL 中，声明 layout(location = 0) in mat4 modelMatrix; 本质上是编译器的一种语法糖。它会自动占用 0、1、2 和 3 号插槽。
				//! 虽然为了数学计算方便，语言允许你将其视为一个整体矩阵，但在硬件层面，它仍然被视作四个独立的四维向量。
				//! 
				//! 在固定函数渲染管线（OpenGL 1.x）时代，还没有现代意义上的“属性（Attributes）”概念，而是为 GL_VERTEX_ARRAY 、 GL_NORMAL_ARRAY 等数据预留了特定插槽。
				//! 随着可编程管线的出现，工程师需要一种通用的数据传递方式。
				//! 他们之所以沿用 4 分量限制，是为了保持与现有硬件设计的向后兼容性，因为当时的硬件已经针对 $(x, y, z, w)$ 运算进行了深度优化。

				int count = element.GetComponentCount();

				for (int index = 0; index < count; index++) {
					glVertexArrayAttribFormat(
						m_VertexArrayID,
						element.LocationIndex + index,
						element.GetComponentCount(),
						ShaderDataTypeToBaseType(element.Type),
						element.Normalized,
						element.GetComponentCount() * index
					);
				}

			}

			// Format，IFormat，LFormat（这些API的关键是它们读取多少bit，它们如何看待原始bit，它们如何转换原始bit，它们会返回给shader什么）
			//				读取多少bit				如何解读bit				如何转换			返回shader
			// Format		取决于type参数			取决于type参数			含义等价转换		转后后的32bit
			// IFormat		取决于type参数			取决于type参数			含义等价转换		转换后的32bit
			// LFormat		取决于type参数			取决于type参数			含义等价转换		转换后的64bit（拼接两个float寄存器）
			// 
			//! 数据上传VRAM和上传Shader是两个过程
			//! Format不在乎shader中是什么，它只是将产生的bit流放到shader的寄存器里（默认情况下bit流都是带符号的某种数值转换，type为无符号时则无符号转换）
			//! float寄存器和int寄存器是分开的（因为ALU的逻辑不同），注意是寄存器不是显存
			//! GLSL 着色器寄存器几乎无一例外都是 32 位的。无论你输入的是 8 位字节还是 32 位浮点数，硬件在读取数据时都会将其“拓宽（widen）”，从而填满一个 32 位槽位（通常是 IEEE-754 浮点数或 32 位有符号/无符号整数）。
			//! Format内部的转换都是含义等价的转换，255会被转成255.0，Normalize是指在type的数值上下限范围转换到[0 - 1]或者[-1 - 1]


		}

		m_VertexBuffers.push_back(vertexBuffer);

		AYIN_CORE_ASSERT((m_BindingIndex + 1 < maxVAOAttribIndexs), "Exceeds the maximum VAOAttribIndex");
		m_BindingIndex = (m_BindingIndex + 1) % maxVAOAttribIndexs;
	#pragma endregion

	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
	#pragma region DSA
		glVertexArrayElementBuffer(m_VertexArrayID, *indexBuffer);
		m_IndexBuffer = indexBuffer;
	#pragma endregion
	}

}
 