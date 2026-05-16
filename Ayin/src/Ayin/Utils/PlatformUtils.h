#pragma once

#include "Ayin/Core/Core.h"

#include <string>

namespace Ayin {


	struct FileFilter {

		const char* name;		//过滤器名称
		const char* filter;		//过滤器字符串，例如"*.txt;*.docx"

	};

	class AYIN_API FileDialogs
	{

	public:

		//文件弹窗初始化
		static void Init();
		//文件弹窗反初始化
		static void Shutdown();

		//打开文件对话框，返回选择的文件路径
		static std::string OpenFile(std::initializer_list<FileFilter> filters, const char* defaultPath);

		//保存文件对话框，返回选择的文件路径
		static std::string SaveFile(std::initializer_list<FileFilter> filters, const char* defaultPath);

	};

}
