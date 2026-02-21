#pragma once

#include "Ayin/Core.h"
#include <string>


namespace Ayin {

	class AYIN_API Shader {

	public:

		/// <summary>
		/// 编译着色器并生成着色器程序
		/// </summary>
		/// <param name="vertexShaderSrc">顶点着色器源码</param>
		/// <param name="fragmentShaderSrc">片元着色器源码</param>
		Shader(const std::string& vertexShaderSrc, const std::string& fragmentShaderSrc);
		virtual ~Shader();

		/// <summary>
		/// 将着色器程序绑定到当前渲染山下文中
		/// </summary>
		virtual void Bind();

		/// <summary>
		/// 从当前上下文中关闭着色器程序
		/// </summary>
		virtual void UnBind();

	private:
		uint32_t m_ProgramID;
	};

}