#pragma once


#include "Ayin/Renderer/Shader.h"

#include <string>
#include <Glad/glad.h>


namespace Ayin{

	class AYIN_API OpenGLShader : public Shader {

	public:

		/// <summary>
		/// 编译着色器并生成着色器程序
		/// </summary>
		/// <param name="filePath">shader代码路径</param>
		OpenGLShader(const std::string& filePath);

		/// <summary>
		/// 编译着色器并生成着色器程序
		/// </summary>
		/// <param name="vertexShaderSrc">顶点着色器源码</param>
		/// <param name="fragmentShaderSrc">片元着色器源码</param>
		OpenGLShader(const std::string& name, const std::string& vertexShaderSrc, const std::string& fragmentShaderSrc);

		//x 有一说一，我们是否可以构建一个构造器来在运行时动态决定整个着色器执行流程？我觉得这是一个很有意思的构想

		virtual ~OpenGLShader() override;

		virtual inline const std::string& GetName() const override { return m_Name; };

		/// <summary>
		/// 将着色器程序绑定到当前渲染山下文中
		/// </summary>
		virtual void Bind() override;

		/// <summary>
		/// 从当前上下文中关闭着色器程序
		/// </summary>
		virtual void UnBind() override;


		void UploadUniformBool(const std::string& name, bool value);


		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformInt2(const std::string& name, const glm::ivec2& vector);
		void UploadUniformInt3(const std::string& name, const glm::ivec3& vector);
		void UploadUniformInt4(const std::string& name, const glm::ivec4& vector);
		void UploadUniformIntArray(const std::string& name, int* values, uint32_t count);


		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& vector);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& vector);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& vector);


		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
		//? 为什么作个API不暴露给Shader呢（Shader中没有这个接口）
		//! 不同的API的机制是不同的，vulkan和D3D是数据驱动而非OpenGL的状态设置
		//! 简单来说UploadUniformMat4(const std::string& name, const glm::mat4& matrix)这样的接口设计（参数）对OpenGL是容易实现的
		//! 但是对于另外两者，它们以内存映射为主，不太“习惯”通过名称的查询；当然，我们也有手段来实现，让其模仿OpenGL那样就行，但这可能很低效
		//! 所以运行机制上的不同导致很难设计出统一的方法，就更不可能上升到抽象层中了
		//! 但是数据设置需求是存在的，“所谓解耦就是把耦合换个地方而已，因为该做的事情一件也不会减少”
		//! 之后我们会将数据交由材质和渲染器管理（shader本身所需数据和场景数据），数据的传输也是如此，会根据API的不同来手动转换指针，并发起数据传输
		//! 这些过程会被封装在核心内部，使用方不应干涉
		//TODO 不过我仍旧认为shader中应该设计一个统一传输函数接口，毕竟我自己的构想里，shader会通过反射以感知布局信息，材质也将从布局中获得信息


		virtual inline void SetInt(const std::string& name, int value) override { UploadUniformInt(name, value); };
		virtual inline void SetIntArray(const std::string& name, int* values, uint32_t count) { UploadUniformIntArray(name, values, count); };
		virtual inline void SetFloat(const std::string& name, float value) override { UploadUniformFloat(name, value); };
		virtual inline void SetFloat2(const std::string& name, const glm::vec2& vertex) override { UploadUniformFloat2(name, vertex); };
		virtual inline void SetFloat3(const std::string& name, const glm::vec3& vertex) override { UploadUniformFloat3(name, vertex); };
		virtual inline void SetFloat4(const std::string& name, const glm::vec4& vertex) override { UploadUniformFloat4(name, vertex); };
		virtual inline void SetMat3(const std::string& name, const glm::mat3& matrix) override { UploadUniformMat3(name, matrix); };
		virtual inline void SetMat4(const std::string& name, const glm::mat4& matrix) override { UploadUniformMat4(name, matrix); };


	private:

		std::string ReadFile(const std::string& filePath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSource);

	private:

		uint32_t m_ProgramID = 0;		
		std::string m_Name;				// 如果来源于统一的glsl文件那么名称则使用文件的名称(不允许自定义命名)，如果来自碎片化的整合则可以自定义名称（你也必须给一个命名）

	};

}
