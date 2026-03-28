#include <AyinPch.h>

#include "Platform/OpenGL/OpenGLShader.h"
#include <glm/gtc/type_ptr.hpp>

#include <Glad/glad.h>

#include <fstream>

namespace Ayin {


	#pragma region 辅助方法
	static GLenum ShaderTypeFromString(const std::string& type) {

		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment")
			return GL_FRAGMENT_SHADER;

		AYIN_CORE_ASSERT(false, "Unkonwn shader type!");

		return 0;

	}

	static std::string ShaderTypeToString(GLenum type) {

		switch (type) {

			case(GL_VERTEX_SHADER): return "vertex";
			case(GL_FRAGMENT_SHADER): return "fragment";

		}

		AYIN_CORE_ASSERT(false, "Unkonwn shader type!");

		return "";

	}
	#pragma endregion


	OpenGLShader::OpenGLShader(const std::string& filePath) {

		std::string source = ReadFile(filePath);
		auto shaderShouces = PreProcess(source);
		Compile(shaderShouces);

	}


	OpenGLShader::OpenGLShader(const std::string& vertexShaderSrc, const std::string& fragmentShaderSrc) {

		std::unordered_map<GLenum, std::string> shaderSources;
		shaderSources[GL_VERTEX_SHADER] = vertexShaderSrc;
		shaderSources[GL_FRAGMENT_SHADER] = fragmentShaderSrc;

		Compile(shaderSources);
	}

	std::string OpenGLShader::ReadFile(const std::string& filePath) {

		std::string source;
		std::ifstream in(filePath, std::ios::in | std::ios::binary);

		if (in) {
			//获取代码
			//source.resize(in.rdbuf()->in_avail());
			//in.read(&source[0], source.size());
			//in.close();
			//! 一个Bug，缓冲区不会在初始化时填充，而是在第一次读操作时填充
			//! 所以初始化后的buf里什么都没有，导致source.resize(in.rdbuf()->in_avail());中的长度被设置为0
			
			in.seekg(0, std::ios::end);				//设置输入流读位置（不是读操作）
			source.resize(in.tellg());				//获取输入流读位置
			in.seekg(0, std::ios::beg);				//输入流读位置复位（放置到开头）

			in.read(source.data(), source.size());	//读操作
			
		}
		else {

			AYIN_CORE_ERROR("Could not open file '{0}'", filePath);
			//! 不使用断言是因为我们允许这样的错误发生，window上的缓冲大小比较小，所以有可能因为文本过大而导致无法读取?
			//? 真的这样么，感觉in不至于因为太大一次都不完而报错

		}

		return source;

	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source) {

		std::unordered_map<GLenum, std::string> shaderSources;

		//获取typeToken相关参数
		const char* typeToken = "#Type";
		//? 能使用string么，会有优化上的差异么，比如string不会变为立即数？
		//! string不会发生把数据存到只读区域的优化
		size_t typeTokenLength = strlen(typeToken);

		//代码片段的起始位置（例如 #Type 。。。。。。而不是以标准的#version开头）
		size_t begin_of_Passage = source.find(typeToken, 0);

		while (begin_of_Passage != std::string::npos) {

			size_t end_of_line = source.find_first_of("\r\n", begin_of_Passage);			//行结尾
			//! window中的回车是由\r\n一起组成的，Linux中是\n
			size_t begin_of_Type = begin_of_Passage + typeTokenLength + 1;					//类型开头
			std::string type = source.substr(begin_of_Type, end_of_line - begin_of_Type);	//类型

			size_t begin_of_Shader = source.find_first_not_of("\r\n", end_of_line);			//shader代码开头（#version。。。。。。）
			begin_of_Passage = source.find(typeToken, begin_of_Shader);						//下一个代码片段开头（以#Type开头）

			shaderSources[ShaderTypeFromString(type)] = source.substr(begin_of_Shader, begin_of_Passage - begin_of_Shader);//提取shader代码

			//x 注释不用担心，glsl代码本身就至此类似C++的注释方式
		}

		return shaderSources;
	}


	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources) {

		GLint programID = glCreateProgram();

		std::vector<GLuint> shaderIDs(shaderSources.size());

		#pragma region 着色器编译
		for (auto& kv : shaderSources) {

			const char* shaderSrc = kv.second.data();
			GLuint shader = glCreateShader(kv.first);
			glShaderSource(shader, 1, &shaderSrc, 0);
			//! 不能对函数取&，所以写不出来&(kv.second.data())
			glCompileShader(shader);

			{
				int isCompiled = 0;
				glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
				if (!isCompiled) {
					int maxLength = 0;
					glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

					std::string inforLog;
					inforLog.reserve(maxLength);
					glGetShaderInfoLog(shader, maxLength, &maxLength, inforLog.data());

					glDeleteShader(shader);

					AYIN_CORE_ERROR("{0}", inforLog.data());
					AYIN_ASSERT(false, "ERROR::OpenGLShader::{0}::Compilation Failed", ShaderTypeToString(kv.first));
					return;
				}
			}

			glAttachShader(programID, shader);
			shaderIDs.push_back(shader);
		}

		#pragma endregion


		#pragma region 着色器程序

		glLinkProgram(programID);

		{
			int isLinked = 0;
			glGetProgramiv(programID, GL_LINK_STATUS, &isLinked);

			if (!isLinked) {

				int maxLength = 0;
				glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

				std::string inforLog;
				inforLog.reserve(maxLength);
				glGetProgramInfoLog(programID, maxLength, &maxLength, inforLog.data());

				glDeleteProgram(programID);

				for (auto id : shaderIDs) {
					glDeleteShader(id);
				}

				AYIN_CORE_ERROR("{0}", inforLog.data());
				AYIN_ASSERT(false, "ERROR::OpenGLShader::Shader Link Failed!");
				return;
			}
		}

		//确保无误后赋值
		m_ProgramID = programID;

		for (auto id : shaderIDs) {

			glDetachShader(m_ProgramID, id);
			glDeleteShader(id);
			//? 是否真的有必要删除？在有资产管理系统后或许可以复用，而不是每次都编译，先这样，足够满足需求

		}

		#pragma endregion	

	}


	OpenGLShader::~OpenGLShader() {
		glDeleteProgram(m_ProgramID);
	}

	void OpenGLShader::Bind() {
		glUseProgram(m_ProgramID);
	}

	void OpenGLShader::UnBind() {
		int currentProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

		if (currentProgram == m_ProgramID)
			glUseProgram(0);
	}


	void OpenGLShader::UploadUniformBool(const std::string& name, bool value) {

		int lastProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);

		if (lastProgram != m_ProgramID)
			glUseProgram(m_ProgramID);

		// 之后可以试着将location变量作为成员
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform1i(location, value ? 1 : 0);

		if (lastProgram != m_ProgramID)
			glUseProgram(lastProgram);

	}


	void OpenGLShader::UploadUniformInt(const std::string& name, int value) {


		int lastProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);

		if (lastProgram != m_ProgramID)
			glUseProgram(m_ProgramID);

		// 之后可以试着将location变量作为成员
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform1i(location, value);

		if (lastProgram != m_ProgramID)
			glUseProgram(lastProgram);

	}
	void OpenGLShader::UploadUniformInt2(const std::string& name, const glm::ivec2& vector) {

		int lastProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);

		if (lastProgram != m_ProgramID)
			glUseProgram(m_ProgramID);

		// 之后可以试着将location变量作为成员
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform2iv(location, 1, glm::value_ptr(vector));

		if (lastProgram != m_ProgramID)
			glUseProgram(lastProgram);

	}
	void OpenGLShader::UploadUniformInt3(const std::string& name, const glm::ivec3& vector) {

		int lastProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);

		if (lastProgram != m_ProgramID)
			glUseProgram(m_ProgramID);

		// 之后可以试着将location变量作为成员
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform3iv(location, 1, glm::value_ptr(vector));

		if (lastProgram != m_ProgramID)
			glUseProgram(lastProgram);

	}
	void OpenGLShader::UploadUniformInt4(const std::string& name, const glm::ivec4& vector) {

		int lastProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);

		if (lastProgram != m_ProgramID)
			glUseProgram(m_ProgramID);

		// 之后可以试着将location变量作为成员
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform4iv(location, 1, glm::value_ptr(vector));

		if (lastProgram != m_ProgramID)
			glUseProgram(lastProgram);

	}


	void OpenGLShader::UploadUniformFloat(const std::string& name, float value) {

		int lastProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);

		if (lastProgram != m_ProgramID)
			glUseProgram(m_ProgramID);

		// 之后可以试着将location变量作为成员
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform1f(location, value);

		if (lastProgram != m_ProgramID)
			glUseProgram(lastProgram);


	}
	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& vector) {

		int lastProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);

		if (lastProgram != m_ProgramID)
			glUseProgram(m_ProgramID);

		// 之后可以试着将location变量作为成员
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform2fv(location, 1, glm::value_ptr(vector));

		if (lastProgram != m_ProgramID)
			glUseProgram(lastProgram);


	}
	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& vector) {

		int lastProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);

		if (lastProgram != m_ProgramID)
			glUseProgram(m_ProgramID);

		// 之后可以试着将location变量作为成员
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform3fv(location, 1, glm::value_ptr(vector));

		if (lastProgram != m_ProgramID)
			glUseProgram(lastProgram);


	}
	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& vector) {

		int lastProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);

		if (lastProgram != m_ProgramID)
			glUseProgram(m_ProgramID);

		// 之后可以试着将location变量作为成员
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform4fv(location, 1, glm::value_ptr(vector));

		if (lastProgram != m_ProgramID)
			glUseProgram(lastProgram);

	}


	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) {

		int lastProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);

		if (lastProgram != m_ProgramID)
			glUseProgram(m_ProgramID);

		// 之后可以试着将location变量作为成员
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));

		if (lastProgram != m_ProgramID)
			glUseProgram(lastProgram);

	}
	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{

		int lastProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);

		if (lastProgram != m_ProgramID)
			glUseProgram(m_ProgramID);

		// 之后可以试着将location变量作为成员
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));

		if (lastProgram != m_ProgramID)
			glUseProgram(lastProgram);

	}


}