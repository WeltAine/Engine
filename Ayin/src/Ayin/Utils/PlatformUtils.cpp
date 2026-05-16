#include <AyinPch.h>

#include "Ayin/Utils/PlatformUtils.h"

#include "Ayin/Core/Application.h"
#include "Ayin/Core/Window.h"

#include <nfd.h>
#include <nfd_glfw3.h>


namespace Ayin {

	//nfdu8filteritem_t		文件过滤数据类型
	//nfdopendialogu8args_t	打开的配置参数类型
	//nfdsavedialogu8args_t	保存的配置参数类型

	//static std::unique_ptr<::NFD::Guard> s_NfdGuard;	//没有这个啊，官方的readme中的示例也没有使用这个，为什么要这个？

	void FileDialogs::Init() {
		
		NFD_Init();

		//在调用 glfwInit() 和 NFD_Init() 之后、且打开任何文件对话框之前，
		// 请调用一次以下函数，以告知 NFDe 您的应用程序所使用的 wl_display （如果您的应用程序未使用 Wayland，则此函数不会产生任何效果）：
		NFD_SetDisplayPropertiesFromGLFW();
		
	};
	void FileDialogs::Shutdown() {

		NFD_Quit();
	
	};

	//打开文件对话框，返回选择的文件路径
	std::string FileDialogs::OpenFile(std::initializer_list<FileFilter> filters, const char* defaultPath) {
	
		std::vector<nfdu8filteritem_t> ndfFilters;
		ndfFilters.reserve(filters.size());

		for (FileFilter filter : filters) {
			ndfFilters.emplace_back(filter.name, filter.filter);
		}

		nfdopendialogu8args_t openArg;
		openArg.filterList = ndfFilters.data();
		openArg.filterCount = ndfFilters.size();
		openArg.defaultPath = defaultPath;

		//每次需要显示对话框时，请调用以下函数以获取父窗口句柄，并设置相应的参数
		NFD_GetNativeWindowFromGLFWWindow(static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow()), &openArg.parentWindow);

		nfdu8char_t* outPath;
		nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &openArg);
		if (result == NFD_OKAY)
		{
			NFD_FreePathU8(outPath);//?这是什么
			return outPath;
		}
		else if (result == NFD_CANCEL)
		{
			AYIN_CORE_ASSERT(false, "User pressed cancel.");
		}
		else
		{
			AYIN_CORE_ASSERT(false, "Error: {0}", NFD_GetError());

		}

	};

	//保存文件对话框，返回选择的文件路径
	std::string FileDialogs::SaveFile(std::initializer_list<FileFilter> filters, const char* defaultName) {
	
		std::vector<nfdu8filteritem_t> ndfFilters;
		ndfFilters.reserve(filters.size());

		for (FileFilter filter : filters) {
			ndfFilters.emplace_back(filter.name, filter.filter);
		}

		nfdsavedialogu8args_t saveArg;
		saveArg.filterList = ndfFilters.data();
		saveArg.filterCount = ndfFilters.size();
		saveArg.defaultName = defaultName;
		//defaultPath不设置么

		//两个default是拿来干嘛的？

		//每次需要显示对话框时，请调用以下函数以获取父窗口句柄，并设置相应的参数
		NFD_GetNativeWindowFromGLFWWindow(static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow()), &saveArg.parentWindow);

		nfdu8char_t* outPath;
		nfdresult_t result = NFD_SaveDialogU8_With(&outPath, &saveArg);
		if (result == NFD_OKAY)
		{
			NFD_FreePathU8(outPath);//?这是什么
			return outPath;
		}
		else if (result == NFD_CANCEL)
		{
			AYIN_CORE_ASSERT(false, "User pressed cancel.");
		}
		else
		{
			AYIN_CORE_ASSERT(false, "Error: {0}", NFD_GetError());

		}

	};


}
