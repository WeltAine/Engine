#pragma once

#include "Ayin/Core/Core.h"

#include <glm/glm.hpp>


namespace Ayin {

	//////////////////////////////////////////////////////////////////////////////////////////
	/// Shader ///////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////


	class AYIN_API Shader {

	public:

		/// <summary>
		/// 生成可运行的着色器对象
		/// </summary>
		/// <param name="vertexShaderSrc">着色器源码</param>
		static Ref<Shader> Create(const std::string& filePath);

		/// <summary>
		/// 生成可运行的着色器对象
		/// </summary>
		/// <param name="vertexShaderSrc">顶点着色器源码</param>
		/// <param name="fragmentShaderSrc">片元着色器源码</param>
		static Ref<Shader> Create(const std::string& name, const std::string& vertexShaderSrc, const std::string& fragmentShaderSrc);
		

		virtual ~Shader() = default;


		virtual const std::string& GetName() const = 0;

		/// <summary>
		/// 将着色器程序绑定到当前渲染山下文中
		/// </summary>
		virtual void Bind() = 0;

		/// <summary>
		/// 从当前上下文中关闭着色器程序
		/// </summary>
		virtual void UnBind() = 0;


		//TODO Shader所需数据的设置API（目前没有的原因详见OpenGLShader.h）
		//x 不同的API的机制是不同的，vulkan和D3D是数据驱动而非OpenGL的状态设置
		//x 简单来说UploadUniformMat4(const std::string& name, const glm::mat4& matrix)这样的接口设计（参数）对OpenGL是容易实现的
		//x 但是对于另外两者，它们以内存映射为主，不太“习惯”通过名称的查询；当然，我们也有手段来实现，让其模仿OpenGL那样就行，但这可能很低效
		//x 所以运行机制上的不同导致很难设计出统一的方法，就更不可能上升到抽象层中了
		//x 但是数据设置需求是存在的，“所谓解耦就是把耦合换个地方而已，因为该做的事情一件也不会减少”
		//x 之后我们会将数据交由材质和渲染器管理（shader本身所需数据和场景数据），数据的传输也是如此，会根据API的不同来手动转换指针，并发起数据传输
		//x 这些过程会被封装在核心内部，使用方不应干涉

		virtual void SetInt(const std::string& name, int value) = 0;

		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& vertex) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& vertex) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& vertex) = 0;

		virtual void SetMat3(const std::string& name, const glm::mat3& matrix) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& matrix) = 0;
	};


	//////////////////////////////////////////////////////////////////////////////////////////
	/// ShaderLibrary ////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////


	class ShaderLibrary {


	public:

		/// <summary>
		/// 加载着色器（编译，入库）
		/// </summary>
		/// <param name="filePath"></param>
		/// <returns></returns>
		Ref<Shader> Load(const std::string& filePath);
		Ref<Shader> Load(const std::string& name, const std::string& filePath);

		/// <summary>
		/// 添加着色器到库中
		/// </summary>
		/// <param name="shader"></param>
		void Add(const Ref<Shader>& shader);
		void Add(const std::string& name, const Ref<Shader>& shader);

		inline Ref<Shader> Get(const std::string& name) {
			AYIN_CORE_ASSERT(Exists(name), "No Name_Index was found in the ShaderLibrary");
			return m_Shaders[name]; 
		};

		//考虑到未来底层数据结构可能发生变化，所以留一个接口做收束整合
		bool Exists(const std::string& name) const;

	private:

		std::unordered_map < std::string, Ref<Shader> > m_Shaders;
	};
}