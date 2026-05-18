#pragma once

#include "Ayin/Core/Core.h"

#include <string>
#include <optional>


namespace Ayin {


	struct FileFilter {

		const char* name;		// 过滤器友好名称，如 "场景文件"
		const char* spec;		// 扩展名说明，逗号分隔，如 "hazel,yaml"（不要带点或通配符）

	};

	class AYIN_API FileDialogs
	{

	public:

		//文件弹窗初始化
		static void Init();
		//文件弹窗反初始化
		static void Shutdown();

		//打开文件对话框，返回选择的文件路径
		static std::optional<std::string> OpenFile(std::initializer_list<FileFilter> filters, const char* defaultPath);

		//保存文件对话框，返回选择的保存路径
		static std::optional<std::string> SaveFile(std::initializer_list<FileFilter> filters, const char* defaultName);

	};

}
