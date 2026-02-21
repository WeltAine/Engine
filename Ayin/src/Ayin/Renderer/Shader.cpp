#include <AyinPch.h>

#include "Shader.h"
#include <GLad/glad.h>

// 目前直接作为OpenGL侧的实现（之后会拆出，如果有时间接入其它API）
namespace Ayin {

	Shader::Shader(const std::string& vertexShaderSrc, const std::string& fragmentShaderSrc) {

		#pragma region 顶点着色器
		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		const char* source = vertexShaderSrc.data();
		glShaderSource(vertexShader, 1, &source, nullptr);
		glCompileShader(vertexShader);

		{
			int isCompiled = 0;
			glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
			if (!isCompiled) {
				int maxLength = 0;
				glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

				std::string inforLog;
				inforLog.reserve(maxLength);
				glGetShaderInfoLog(vertexShader, maxLength, &maxLength, inforLog.data());

				AYIN_CORE_ERROR("{0}", inforLog.data());
				AYIN_ASSERT(false, "ERROR::Shader::vertex::Compilation Failed");
				return;
			}
		}
		#pragma endregion


		#pragma region 片段着色器
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		source = fragmentShaderSrc.data();
		glShaderSource(fragmentShader, 1, &source, nullptr);
		glCompileShader(fragmentShader);

		{
			int isCompiled = 0;
			glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
			if (!isCompiled) {
				int maxLength = 0;
				glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

				std::string inforLog;
				inforLog.reserve(maxLength);
				glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, inforLog.data());

				AYIN_CORE_ERROR("{0}", inforLog.data());
				AYIN_ASSERT(false, "ERROR::Shader::fragment::Compilation Failed");
				return;
			}
		}
		#pragma endregion


		#pragma region 着色器程序
		m_ProgramID = glCreateProgram();

		glAttachShader(m_ProgramID, vertexShader);
		glAttachShader(m_ProgramID, fragmentShader);

		glLinkProgram(m_ProgramID);

		{
			int isLinked = 0;
			glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &isLinked);

			if (!isLinked) {

				int maxLength = 0;
				glGetProgramiv(m_ProgramID, GL_INFO_LOG_LENGTH, &maxLength);

				std::string inforLog;
				inforLog.reserve(maxLength);
				glGetProgramInfoLog(m_ProgramID, maxLength, &maxLength, inforLog.data());

				AYIN_CORE_ERROR("{0}", inforLog.data());
				AYIN_ASSERT(false, "ERROR::Shader::Shader Link Failed!");
				return;
			}
		}

		glDetachShader(m_ProgramID, vertexShader);
		glDetachShader(m_ProgramID, fragmentShader);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		#pragma endregion	
	}

	Shader::~Shader() {
		glDeleteProgram(m_ProgramID);
	}

	void Shader::Bind() {
		glUseProgram(m_ProgramID);
	}

	void Shader::UnBind() {
		int currentProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

		if(currentProgram == m_ProgramID)
			glUseProgram(0);
	}

}