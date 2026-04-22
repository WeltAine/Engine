#include <AyinPch.h>

#include "OpenGLBuffer.h"

#include <Glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Ayin {

	//////////////////////////////////////////////////////////////////////////////////////////
	/// VertexBuffer /////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////


	OpenGLVertexBuffer::OpenGLVertexBuffer(size_t size) {

	#pragma region DSA
		glCreateBuffers(1, &m_VertexBufferID);
		////分配 ( $glCreateBuffers$ )：系统生成唯一 ID，GPU 预留句柄，但此时尚未实际分配显存。

		glNamedBufferData(m_VertexBufferID, size, nullptr, GL_STATIC_DRAW);//只开辟空间不设置数据
	#pragma endregion

	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, size_t size) {

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


	void OpenGLVertexBuffer::SetData(const void* data, size_t size) {

	#pragma region DSA
		glNamedBufferSubData(m_VertexBufferID, 0, size, data);
	#pragma endregion


	}

	void OpenGLVertexBuffer::Bind() const { 

	#pragma region BTA/DSA
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID); 
	#pragma endregion

	}

	void OpenGLVertexBuffer::UnBind() const {

	#pragma region BTA/DSA

		int currentVertexBufferID = 0;
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVertexBufferID);

		if (currentVertexBufferID == m_VertexBufferID)
			glBindBuffer(GL_ARRAY_BUFFER, 0);

	#pragma endregion

	}



	//////////////////////////////////////////////////////////////////////////////////////////
	/// IndexBuffer //////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count) 
		: m_Count(count)
	{

	#pragma region BTA（该注释具备参考意义，暂不删除）

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

		//ToDo:区别是什么？后者不可再重开内存？
		//glNamedBufferData(buffer, size, data, usage);
		//glNamedBufferStorage(buffer, size, data, flags);

		//glNamedBufferSubData();

	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer() {

	#pragma region BTA/DSA
		UnBind();
		glDeleteBuffers(1, &m_IndexBufferID);
	#pragma endregion

	}

	void OpenGLIndexBuffer::Bind() const { 

	#pragma region BTA/DSA
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferID); 
	#pragma endregion

	}

	void OpenGLIndexBuffer::UnBind() const {
	
	#pragma region BTA/DSA

		int currentIndexBufferID;
		glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &currentIndexBufferID);

		if (currentIndexBufferID == m_IndexBufferID)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	#pragma endregion

	}


	//////////////////////////////////////////////////////////////////////////////////////////
	/// UniformBuffer ////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	//ToDo 先这么写着，之后可以高层次Init，内部再低层次Init来针对不同API初始化，就像Renderer里的那样
	std::vector<int> OpenGLUniformBuffer::s_BindingIndexs(64);


	OpenGLUniformBuffer::OpenGLUniformBuffer(void* uniformData, size_t size)
	{

		//glBufferData(目标，大小，数据，用法);
		//glBindBufferBase(目标，索引（bind）, 对象);	//也可以不指定（注意不会设置当前活跃的UBO，只是指定，shader中没有这个block块也没关系）
		//glBufferSubData(目标, 起始偏移, 大小，数据);//向当前活跃对象上传数据，偏移是GPU中block内的偏移
		//glBindBufferRange(目标，索引（bind），对象，起始偏移，总大小);//划分对象数据段到指定bind

	#pragma region DSA
		glCreateBuffers(1, &m_UniformBufferID);
		glNamedBufferData(m_UniformBufferID, size, uniformData, GL_DYNAMIC_DRAW);
	#pragma endregion

	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer() {

	#pragma region BTA/DSA
		UnBind();

		//解除相关绑定
		for (int index = 0; index < s_BindingIndexs.size(); index++) {
			if (s_BindingIndexs[index] = m_UniformBufferID)
				glBindBufferBase(GL_UNIFORM_BUFFER, index, 0);
		}
			
		glDeleteBuffers(1, &m_UniformBufferID);
	#pragma endregion

	}

	void OpenGLUniformBuffer::Set(void* data)
	{
	#pragma region DSA
		// 设置UBO对应数据块整个区域数据
		glNamedBufferSubData(m_UniformBufferID, 0, m_UniformLayout.GetSize(), data);
	#pragma endregion
	}

	void OpenGLUniformBuffer::SetBindingIndexs(const std::initializer_list<int>& bindingIndexs) {

	#pragma region BTA/DSA
		for (const int& index : bindingIndexs) {
			s_BindingIndexs[index] = m_UniformBufferID ;
			glBindBufferBase(GL_UNIFORM_BUFFER, index, m_UniformBufferID);		// UBO绑定到上下文的对应BlockBinding上
		}
	#pragma endregion

	}


	void OpenGLUniformBuffer::Bind() const {

	#pragma region BTA/DSA
		glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferID);
	#pragma endregion

	}


	void OpenGLUniformBuffer::UnBind() const
	{
	#pragma region BTA/DSA
		int currentUniformBufferID = 0;
		glGetIntegerv(GL_UNIFORM_BUFFER, &currentUniformBufferID);

		if (currentUniformBufferID == m_UniformBufferID)
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

	#pragma endregion
	}





	void OpenGLUniformBuffer::UploadBool(const std::string& paramName, bool value) {

	#pragma region DSA
		auto it = std::find_if(m_UniformLayout.begin(), m_UniformLayout.end(), [&paramName](const UniformElement& element) {
			return element.Name == paramName;
			});
		AYIN_ASSERT(it != m_UniformLayout.end(), "Unfind {0} in shader!", paramName);


		glNamedBufferSubData(m_UniformBufferID, it->Offset, it->Size, &value);
	#pragma endregion

	}


	void OpenGLUniformBuffer::UploadInt(const std::string& paramName, int value) {

	#pragma region DSA
		auto it = std::find_if(m_UniformLayout.begin(), m_UniformLayout.end(), [&paramName](const UniformElement& element) {
			return element.Name == paramName;
			});
		AYIN_ASSERT(it != m_UniformLayout.end(), "Unfind {0} in shader!", paramName);


		glNamedBufferSubData(m_UniformBufferID, it->Offset, it->Size, &value);
	#pragma endregion

	}
	void OpenGLUniformBuffer::UploadInt2(const std::string& paramName, const glm::ivec2& vector) {

	#pragma region DSA
		auto it = std::find_if(m_UniformLayout.begin(), m_UniformLayout.end(), [&paramName](const UniformElement& element) {
			return element.Name == paramName;
			});
		AYIN_ASSERT(it != m_UniformLayout.end(), "Unfind {0} in shader!", paramName);


		glNamedBufferSubData(m_UniformBufferID, it->Offset, it->Size, glm::value_ptr(vector));
	#pragma endregion
	
	}
	void OpenGLUniformBuffer::UploadInt3(const std::string& paramName, const glm::ivec3& vector) {

	#pragma region DSA
		auto it = std::find_if(m_UniformLayout.begin(), m_UniformLayout.end(), [&paramName](const UniformElement& element) {
			return element.Name == paramName;
			});
		AYIN_ASSERT(it != m_UniformLayout.end(), "Unfind {0} in shader!", paramName);


		glNamedBufferSubData(m_UniformBufferID, it->Offset, it->Size, glm::value_ptr(vector));
	#pragma endregion

	}
	void OpenGLUniformBuffer::UploadInt4(const std::string& paramName, const glm::ivec4& vector) {

	#pragma region DSA
		auto it = std::find_if(m_UniformLayout.begin(), m_UniformLayout.end(), [&paramName](const UniformElement& element) {
			return element.Name == paramName;
			});
		AYIN_ASSERT(it != m_UniformLayout.end(), "Unfind {0} in shader!", paramName);


		glNamedBufferSubData(m_UniformBufferID, it->Offset, it->Size, glm::value_ptr(vector));
	#pragma endregion

	}


	void OpenGLUniformBuffer::UploadFloat(const std::string& paramName, float value) {

	#pragma region DSA
		auto it = std::find_if(m_UniformLayout.begin(), m_UniformLayout.end(), [&paramName](const UniformElement& element) {
			return element.Name == paramName;
			});
		AYIN_ASSERT(it != m_UniformLayout.end(), "Unfind {0} in shader!", paramName);


		glNamedBufferSubData(m_UniformBufferID, it->Offset, it->Size, &value);
	#pragma endregion

	}
	void OpenGLUniformBuffer::UploadFloat2(const std::string& paramName, const glm::vec2& vector) {
	
	#pragma region DSA
		auto it = std::find_if(m_UniformLayout.begin(), m_UniformLayout.end(), [&paramName](const UniformElement& element) {
			return element.Name == paramName;
			});
		AYIN_ASSERT(it != m_UniformLayout.end(), "Unfind {0} in shader!", paramName);


		glNamedBufferSubData(m_UniformBufferID, it->Offset, it->Size, glm::value_ptr(vector));
	#pragma endregion

	}
	void OpenGLUniformBuffer::UploadFloat3(const std::string& paramName, const glm::vec3& vector) {

	#pragma region DSA
		auto it = std::find_if(m_UniformLayout.begin(), m_UniformLayout.end(), [&paramName](const UniformElement& element) {
			return element.Name == paramName;
			});
		AYIN_ASSERT(it != m_UniformLayout.end(), "Unfind {0} in shader!", paramName);


		glNamedBufferSubData(m_UniformBufferID, it->Offset, it->Size, glm::value_ptr(vector));
	#pragma endregion

	}
	void OpenGLUniformBuffer::UploadFloat4(const std::string& paramName, const glm::vec4& vector) {

	#pragma region DSA
		auto it = std::find_if(m_UniformLayout.begin(), m_UniformLayout.end(), [&paramName](const UniformElement& element) {
			return element.Name == paramName;
			});
		AYIN_ASSERT(it != m_UniformLayout.end(), "Unfind {0} in shader!", paramName);


		glNamedBufferSubData(m_UniformBufferID, it->Offset, it->Size, glm::value_ptr(vector));
	#pragma endregion

	}


	void OpenGLUniformBuffer::UploadMat3(const std::string& paramName, const glm::mat3& matrix) {

	#pragma region DSA
		auto it = std::find_if(m_UniformLayout.begin(), m_UniformLayout.end(), [&paramName](const UniformElement& element) {
			return element.Name == paramName;
			});
		AYIN_ASSERT(it != m_UniformLayout.end(), "Unfind {0} in shader!", paramName);


		glNamedBufferSubData(m_UniformBufferID, it->Offset, it->Size, glm::value_ptr(matrix));
	#pragma endregion

	}
	void OpenGLUniformBuffer::UploadMat4(const std::string& paramName, const glm::mat4& matrix) {

	#pragma region DSA
		auto it = std::find_if(m_UniformLayout.begin(), m_UniformLayout.end(), [&paramName](const UniformElement& element) {
			return element.Name == paramName;
			});
		AYIN_ASSERT(it != m_UniformLayout.end(), "Unfind {0} in shader!", paramName);


		glNamedBufferSubData(m_UniformBufferID, it->Offset, it->Size, glm::value_ptr(matrix));
	#pragma endregion

	}



}