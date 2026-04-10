#include <AyinPch.h>
#include "OpenGLBuffer.h"

#include <Glad/glad.h>

namespace Ayin {

	//////////////////////////////////////////////////////////////////////////////////////////
	/// VertexBuffer /////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, size_t size) {

	#pragma region BTA

		//int currentVertexBufferID = 0;
		//glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVertexBufferID);

		//glCreateBuffers(1, &m_VertexBufferID);
		//glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID);
		//glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

		//glBindBuffer(GL_ARRAY_BUFFER, currentVertexBufferID);

	#pragma endregion


	#pragma region DSA

		glCreateBuffers(1, &m_VertexBufferID);
		glNamedBufferData(m_VertexBufferID, size, vertices, GL_STATIC_DRAW);

	#pragma endregion

	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer() {

	#pragma region BTA/DSA
		UnBind();
		glDeleteBuffers(1, &m_VertexBufferID);
	#pragma endregion

	}

	void OpenGLVertexBuffer::Bind() const { 

	#pragma region BTA/DSA
		//glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID); 
	#pragma endregion

	#pragma region DSA
		//nothing
	#pragma endregion

	}

	void OpenGLVertexBuffer::UnBind() const {

	#pragma region BTA/DSA

		//int currentVertexBufferID = 0;
		//glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVertexBufferID);

		//if (currentVertexBufferID == m_VertexBufferID)
		//	glBindBuffer(GL_ARRAY_BUFFER, 0);

	#pragma endregion

	#pragma region DSA
		//nothing
	#pragma endregion

	}



	//////////////////////////////////////////////////////////////////////////////////////////
	/// IndexBuffer //////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count) 
		: m_Count(count)
	{

	#pragma region BTA

		//int currentVertexBufferID = 0;
		//glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVertexBufferID);

		//glCreateBuffers(1, &m_IndexBufferID);
		//glBindBuffer(GL_ARRAY_BUFFER, m_IndexBufferID);
		//glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
		////? 为什么设置为GL_ARRAY_BUFFER，而不是GL_ELEMENT_ARRAY_BUFFER
		////! 因为OpenGL中全局VAO会自动去绑定VBO（在设置顶点属性指针时）和EBO（在bind时）；
		////! 而EVO槽只能有一个，这意味着我们的glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferID);可能会覆盖VAO原本的VEO；
		////! 正如我们在 OpenGLBuffer.h 开头所警告的那样，我们不应当让这样的事情发生
		////! 所以我们设置到GL_ARRAY_BUFFER，以避免触发覆盖EBO（当然你也可以写一段记录代码，之后用于恢复，但使用GL_ARRAY_BUFFER更快捷）
		////? 设置为顶点属性后怎么作为索引使用？
		////! 直接glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferID); 就好，就会视为EBO使用，而这由bind来完成。
		////! 缓冲对象（内存）：一段原始的 GPU 显存。对 GPU 而言，显存本身并不区分是“顶点”数据还是“索引”数据；
		////! 所谓的“目标（Target）”仅仅是我们与其进行交互的一种视角或方式。

		//glBindBuffer(GL_ARRAY_BUFFER, currentVertexBufferID);

	#pragma endregion



	#pragma region DSA

		glCreateBuffers(1, &m_IndexBufferID);
		glNamedBufferData(m_IndexBufferID, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);

	#pragma endregion


		//DSA

		//! 

		////设置a_index和b_index的链接关系
		//glVertexArrayAttribBinding(vao, a_index, b_index);
		////设置a_index的读取模式
		//glVertexArrayAttribFormat(vao, a_index, size, type, normalized, offset);
		////设置b_index的shader上传模式
		//glVertexArrayBindingDivisor(vao, b_index, divisor);//不是glVertexBufferBindingDivisor
		////设置b_index对应的buffer对象，shader数据上传方式?
		//glVertexArrayVertexBuffer(vao, b_index, buffer, offset, stride);

		//

		//glBindVertexBuffer(b_index, buffer, offset, stride);

		////区别是什么？后者不可再重开内存？
		//glNamedBufferData(buffer, size, data, usage);
		//glNamedBufferStorage(buffer, size, data, flags);

		//glNamedBufferSubData();

		//glCreateBuffers();
		////分配 ( $glCreateBuffers$ )：系统生成唯一 ID，GPU 预留句柄，但此时尚未实际分配显存。
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer() {

	#pragma region BTA/DSA
		UnBind();
		glDeleteBuffers(1, &m_IndexBufferID);
	#pragma endregion

	}

	void OpenGLIndexBuffer::Bind() const { 

	#pragma region BTA
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferID); 
	#pragma endregion

	#pragma region DSA
		//nothing
	#pragma endregion

	}

	void OpenGLIndexBuffer::UnBind() const {
	
	#pragma region BTA

		//int currentIndexBufferID;
		//glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &currentIndexBufferID);

		//if (currentIndexBufferID == m_IndexBufferID)
		//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	#pragma endregion

	#pragma region DSA
		//nothing
	#pragma endregion

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

		int maxUBOBindings = 0;
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUBOBindings);

		m_BindingIndexs.reserve(maxUBOBindings);


	#pragma region BTA
		//int currentUniformBufferID = 0;
		//glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &currentUniformBufferID);

		//// 分配总显存大小
		//glCreateBuffers(1, &m_UniformBufferID);
		//glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferID);
		//glBufferData(GL_UNIFORM_BUFFER, size, uniformData, GL_DYNAMIC_DRAW);	// 之后移动走，不由Uniform、开辟，由Shader统一开辟


		//glBindBuffer(GL_UNIFORM_BUFFER, currentUniformBufferID);
	#pragma endregion

	#pragma region DSA
		glCreateBuffers(1, &m_UniformBufferID);
		glNamedBufferData(m_UniformBufferID, size, uniformData, GL_DYNAMIC_DRAW);
	#pragma endregion

	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer() {

	#pragma region BTA/DSA
		UnBind();
		glDeleteBuffers(1, &m_UniformBufferID);
	#pragma endregion

	}


	void OpenGLUniformBuffer::Set(const std::string& paramName, void* data)
	{
		auto it = std::find_if(m_UniformLayout.begin(), m_UniformLayout.end(), [&paramName](const UniformElement& element) {
			return element.Name == paramName;
			});

		AYIN_ASSERT(it != m_UniformLayout.end(), "Unfind {0} in shader!", paramName);

	#pragma region BTA
		//int currentUniformBufferID = 0;
		//glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &currentUniformBufferID);

		//glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferID);

		//// 设置UBO对应数据块上某一段区域数据
		//glBufferSubData(GL_UNIFORM_BUFFER, it->Offset, it->Size, data);

		//glBindBuffer(GL_UNIFORM_BUFFER, currentUniformBufferID);
	#pragma endregion


	#pragma region DSA
		// 设置UBO对应数据块上某一段区域数据
		glNamedBufferSubData(m_UniformBufferID, it->Offset, it->Size, data);
	#pragma endregion

	}

	void OpenGLUniformBuffer::Set(void* data)
	{
	#pragma region BTA
		//int currentUniformBufferID = 0;
		//glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &currentUniformBufferID);

		//glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferID);

		//// 设置UBO对应数据块整个区域数据
		//glBufferSubData(GL_UNIFORM_BUFFER, 0, m_UniformLayout.GetSize(), data);

		//glBindBuffer(GL_UNIFORM_BUFFER, currentUniformBufferID);
	#pragma endregion



	#pragma region DSA
		// 设置UBO对应数据块整个区域数据
		glNamedBufferSubData(m_UniformBufferID, 0, m_UniformLayout.GetSize(), data);
	#pragma endregion
	}

	void OpenGLUniformBuffer::SetBindingIndexs(const std::initializer_list<int>& bindingIndexs) {

	#pragma region BTA/DSA
		m_BindingIndexs = bindingIndexs;
	#pragma endregion

	}


	void OpenGLUniformBuffer::Bind() const {

	#pragma region BTA
		//glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferID);

		////清除旧绑定
		//int maxUBOBindings = 0;
		//glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUBOBindings);

		//int indexToUniformBufferID = 0;
		//for (int index = 0; index < maxUBOBindings; index++) {

		//	glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, index, &indexToUniformBufferID);

		//	if (indexToUniformBufferID == m_UniformBufferID)
		//		glBindBufferBase(GL_UNIFORM_BUFFER, index, 0);
		//}

		////建立新绑定
		//for (int index : m_BindingIndexs) {
		//	glBindBufferBase(GL_UNIFORM_BUFFER, index, m_UniformBufferID);
		//}
	#pragma endregion

	#pragma region DSA
		//清除旧绑定
		int maxUBOBindings = 0;
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUBOBindings);

		int indexToUniformBufferID = 0;
		for (int index = 0; index < maxUBOBindings; index++) {

			glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, index, &indexToUniformBufferID);

			if (indexToUniformBufferID == m_UniformBufferID)
				glBindBufferBase(GL_UNIFORM_BUFFER, index, 0);
		}

		//建立新绑定
		for (int index : m_BindingIndexs) {
			glBindBufferBase(GL_UNIFORM_BUFFER, index, m_UniformBufferID);
		}
	#pragma endregion

	}

	void OpenGLUniformBuffer::Bind(int blockIndex)
	{
	#pragma region BTA
		//glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferID);
		//glBindBufferBase(GL_UNIFORM_BUFFER, blockIndex, m_UniformBufferID);	// UBO绑定到上下文的对应BlockBinding上
		//m_BindingIndexs.push_back(blockIndex)
	#pragma endregion

	#pragma region DSA
		glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferID);						//! 尽管对于让Shader使用上正确的UBO来说并不必要，但还是写上，因为Bind方法有这层含义
		glBindBufferBase(GL_UNIFORM_BUFFER, blockIndex, m_UniformBufferID);		// UBO绑定到上下文的对应BlockBinding上
		m_BindingIndexs.push_back(blockIndex);
	#pragma endregion
	}


	void OpenGLUniformBuffer::UnBind() const
	{
	#pragma region BTA
		//int currentUniformBufferID = 0;
		//glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &currentUniformBufferID);

		//int maxUBOBindings = 0;
		//glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUBOBindings);

		//int indexToUniformBufferID = 0;
		//for (int index = 0; index < maxUBOBindings; index++) {

		//	glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, index, &indexToUniformBufferID);

		//	if (indexToUniformBufferID == m_UniformBufferID)
		//		glBindBufferBase(GL_UNIFORM_BUFFER, index, 0);
		//}

		//if (currentUniformBufferID == m_UniformBufferID)
		//	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	#pragma endregion



	#pragma region DSA
		int maxUBOBindings = 0;
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUBOBindings);

		int indexToUniformBufferID = 0;
		for (int index = 0; index < maxUBOBindings; index++) {

			glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, index, &indexToUniformBufferID);

			if (indexToUniformBufferID == m_UniformBufferID)
				glBindBufferBase(GL_UNIFORM_BUFFER, index, 0);
		}
	#pragma endregion
	}


}