#include <AyinPch.h>

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/type_ptr.hpp>

#include <Glad/glad.h>

#include <fstream>

namespace Ayin {


	#pragma region 辅助方法（仅限于本文件使用）
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

		AYIN_PROFILE_FUNCTION();

		// 查找名称
		size_t lastSlash = filePath.find_last_of("/\\");//找"/"或者"\"
		//! 该方法的含义为，查找字符串中最后出现过参数字符串中任意一个字符的位置（就像拿着一系列嫌疑人画像（参数），在监控里（对象）查找最后一个出现的嫌疑人）
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;

		size_t lastDot = filePath.rfind('.');//查找最后出现的目标（完全匹配）
		lastDot = lastDot == std::string::npos ? filePath.size() : lastDot;

		size_t count = lastDot - lastSlash;

		m_Name = filePath.substr(lastSlash, count);

		// 编译
		std::string source = ReadFile(filePath);
		auto shaderShouces = PreProcess(source);
		Compile(shaderShouces);

	}


	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexShaderSrc, const std::string& fragmentShaderSrc) 
		:m_Name{name}
	{
		AYIN_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> shaderSources;
		shaderSources[GL_VERTEX_SHADER] = vertexShaderSrc;
		shaderSources[GL_FRAGMENT_SHADER] = fragmentShaderSrc;

		Compile(shaderSources);
	}

	std::string OpenGLShader::ReadFile(const std::string& filePath) {

		AYIN_PROFILE_FUNCTION();

		std::string source;
		std::ifstream in(filePath, std::ios::in | std::ios::binary);

		if (in) {
			//获取代码
			//source.resize(in.rdbuf()->in_avail());
			//in.read(&source[0], source.size());
			//in.close();
			//! 一个Bug，缓冲区不会在初始化时填充，而是在第一次读操作时填充
			//! 所以初始化后的buf里什么都没有，导致source.resize(in.rdbuf()->in_avail());中的长度被设置为0
			
			in.seekg(0, std::ios::end);					//设置输入流读位置（不是读操作）
			size_t fileSize = in.tellg();

			if (fileSize != -1) {							//之前的寻找文件末尾的操作可能失败，或者发现文件大到超过存储上限时不要读取内容
			
				source.resize(fileSize);				//获取输入流读位置
				in.seekg(0, std::ios::beg);				//输入流读位置复位（放置到开头）

				in.read(source.data(), source.size());	//读操作

			}
			else {
				AYIN_CORE_ERROR("Could not read file '{0}'", filePath);
			}

		}
		else {

			AYIN_CORE_ERROR("Could not open file '{0}'", filePath);

		}

		return source;

	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source) {

		AYIN_PROFILE_FUNCTION();

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
			AYIN_CORE_ASSERT(end_of_line != std::string::npos, "Syntax error");

			size_t begin_of_Type = begin_of_Passage + typeTokenLength + 1;					//类型开头
			std::string type = source.substr(begin_of_Type, end_of_line - begin_of_Type);	//类型
			AYIN_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified");

			size_t begin_of_Shader = source.find_first_not_of("\r\n", end_of_line);			//shader代码开头（#version。。。。。。）
			begin_of_Passage = source.find(typeToken, begin_of_Shader);						//下一个代码片段开头（以#Type开头）

			shaderSources[ShaderTypeFromString(type)] = source.substr(begin_of_Shader, begin_of_Passage - begin_of_Shader);//提取shader代码
			//! string中能够存储的最大大小就是npos，这是极限，不允许超过这么大
			//! 所以我们不必再做一个关于npos的判定，因为即使没有下一段代码(或者说已经读到最后一段了)，那么本次获取的范围只有两种可能
			//! 1) 获取从begin_of_Shader开始超出文本剩余长度的文本内容 （会自然的读取完剩余内容）
			//! 2)获取从begin_of_Shader开始到npos的文本内容 （这种情况只在string的文本内容完全填满上限时发生，这也意味着我们没能读完）

			//x 注释不用担心，glsl代码本身就支持类似C++的注释方式
		}

		return shaderSources;
	}


	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources) {

		AYIN_PROFILE_FUNCTION();

		GLint programID = glCreateProgram();

		//std::vector<GLuint> shaderIDs(shaderSources.size());
		////! 该方法会开辟并填充空间
		AYIN_CORE_ASSERT((shaderSources.size() <= 2), "Only two shaders are currently supported");
		std::array<int, 2> shaderIDs;

		int glShaderIDIndex = 0;
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
					AYIN_CORE_ASSERT(false, "ERROR::OpenGLShader::{0}::Compilation Failed", ShaderTypeToString(kv.first).data());
					return;
				}
			}

			glAttachShader(programID, shader);
			shaderIDs[glShaderIDIndex++] = shader;
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
				AYIN_CORE_ASSERT(false, "ERROR::OpenGLShader::Shader Link Failed!");
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

		AYIN_PROFILE_FUNCTION();

		glDeleteProgram(m_ProgramID);
	}

	void OpenGLShader::Bind() {

		AYIN_PROFILE_FUNCTION();

		glUseProgram(m_ProgramID);
	}

	void OpenGLShader::UnBind() {

		AYIN_PROFILE_FUNCTION();

		int currentProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

		if (currentProgram == m_ProgramID)
			glUseProgram(0);
	}


	void OpenGLShader::UploadUniformBool(const std::string& name, bool value) {

		AYIN_PROFILE_FUNCTION();

	#pragma region DSA
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glProgramUniform1i(m_ProgramID, location, value);
	#pragma endregion
	}


	void OpenGLShader::UploadUniformInt(const std::string& name, int value) {

		AYIN_PROFILE_FUNCTION();

	#pragma region DSA
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glProgramUniform1i(m_ProgramID, location,value);
	#pragma endregion
	}
	void OpenGLShader::UploadUniformInt2(const std::string& name, const glm::ivec2& vector) {

		AYIN_PROFILE_FUNCTION();

	#pragma region DSA
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glProgramUniform2iv(m_ProgramID, location, 1, glm::value_ptr(vector));
	#pragma endregion
	}
	void OpenGLShader::UploadUniformInt3(const std::string& name, const glm::ivec3& vector) {

		AYIN_PROFILE_FUNCTION();

	#pragma region DSA
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glProgramUniform3iv(m_ProgramID, location, 1, glm::value_ptr(vector));
	#pragma endregion

	}
	void OpenGLShader::UploadUniformInt4(const std::string& name, const glm::ivec4& vector) {

		AYIN_PROFILE_FUNCTION();

	#pragma region DSA
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glProgramUniform4iv(m_ProgramID, location, 1, glm::value_ptr(vector));
	#pragma endregion

	}


	void OpenGLShader::UploadUniformFloat(const std::string& name, float value) {

		AYIN_PROFILE_FUNCTION();

	#pragma region DSA
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glProgramUniform1f(m_ProgramID, location, value);
	#pragma endregion

	}
	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& vector) {

		AYIN_PROFILE_FUNCTION();

	#pragma region DSA
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glProgramUniform2fv(m_ProgramID, location, 1, glm::value_ptr(vector));
	#pragma endregion

	}
	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& vector) {

		AYIN_PROFILE_FUNCTION();

	#pragma region DSA
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glProgramUniform3fv(m_ProgramID, location, 1, glm::value_ptr(vector));
	#pragma endregion
	}
	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& vector) {

		AYIN_PROFILE_FUNCTION();

	#pragma region DSA
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glProgramUniform4fv(m_ProgramID, location, 1, glm::value_ptr(vector));
	#pragma endregion
	}


	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) {

		AYIN_PROFILE_FUNCTION();

	#pragma region DSA
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glProgramUniformMatrix3fv(m_ProgramID, location, 1, GL_FALSE, glm::value_ptr(matrix));
	#pragma endregion

	}
	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		AYIN_PROFILE_FUNCTION();

	#pragma region DSA
		int location = glGetUniformLocation(m_ProgramID, name.c_str());
		glProgramUniformMatrix4fv(m_ProgramID, location, 1, GL_FALSE, glm::value_ptr(matrix));
	#pragma endregion
	}


}