#include "AyinPch.h"

#include "Ayin/Utils/PlatformUtils.h"

#include "Ayin/Core/Application.h"
#include "Ayin/Core/Window.h"

#include <nfd.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <nfd_glfw3.h>

#include <vector>

namespace Ayin {
	//nfdu8filteritem_t		文件过滤数据类型
	//nfdopendialogu8args_t	打开的配置参数类型
	//nfdsavedialogu8args_t	保存的配置参数类型


	void FileDialogs::Init()
	{
		NFD_Init();
		//在调用 glfwInit() 和 NFD_Init() 之后、且打开任何文件对话框之前，
		// 请调用一次以下函数，以告知 NFDe 您的应用程序所使用的 wl_display （如果您的应用程序未使用 Wayland，则此函数不会产生任何效果）：
		NFD_SetDisplayPropertiesFromGLFW();
	}

	void FileDialogs::Shutdown()
	{
		NFD_Quit();
	}

	std::string FileDialogs::OpenFile(std::initializer_list<FileFilter> filters, const char* defaultPath)
	{

		std::vector<nfdu8filteritem_t> nfdFilters;
		nfdFilters.reserve(filters.size());
		for (const FileFilter& filter : filters) {
			nfdFilters.push_back({filter.name, filter.spec});
		}

		//打开参数配置
		nfdopendialogu8args_t openArg = {};
		openArg.filterList = nfdFilters.data();
		openArg.filterCount = static_cast<nfdfiltersize_t>(nfdFilters.size());
		openArg.defaultPath = defaultPath;
		//每次需要显示对话框时，请调用以下函数以获取父窗口句柄，并设置相应的参数
		NFD_GetNativeWindowFromGLFWWindow(
			static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow()),
			&openArg.parentWindow);

		nfdu8char_t* outPath = nullptr;
		switch (NFD_OpenDialogU8_With(&outPath, &openArg)) {
			case NFD_OKAY: {
				std::string result(outPath);
				NFD_FreePathU8(outPath);
				return result;
			}
			case NFD_CANCEL:
				return {};
			default:
				AYIN_CORE_ERROR("File open error: {0}", NFD_GetError());
				return {};
		}

	}

	std::string FileDialogs::SaveFile(std::initializer_list<FileFilter> filters, const char* defaultName)
	{

		std::vector<nfdu8filteritem_t> nfdFilters;
		nfdFilters.reserve(filters.size());
		for (const FileFilter& filter : filters) {
			nfdFilters.push_back({filter.name, filter.spec});
		}

		//保存参数配置
		nfdsavedialogu8args_t saveArg = {};
		saveArg.filterList = nfdFilters.data();
		saveArg.filterCount = static_cast<nfdfiltersize_t>(nfdFilters.size());
		saveArg.defaultName = defaultName;
		NFD_GetNativeWindowFromGLFWWindow(
			static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow()),
			&saveArg.parentWindow);

		nfdu8char_t* outPath = nullptr;
		switch (NFD_SaveDialogU8_With(&outPath, &saveArg)) {
			case NFD_OKAY: {
				std::string result(outPath);
				NFD_FreePathU8(outPath);
				return result;
			}
			case NFD_CANCEL:
				return {};
			default:
				AYIN_CORE_ERROR("File save error: {0}", NFD_GetError());
				return {};
		}

	}

}
