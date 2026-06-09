#include "AyinPch.h"

#include "Ayin/Utils/PlatformUtils.h"

#include "Ayin/Core/Application.h"
#include "Ayin/Core/Window.h"

#include <nfd.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <nfd_glfw3.h>

#include <imm.h>

#pragma comment(lib, "Imm32.lib") // MSVC 的一种“在源码里告诉链接器要链接哪个 .lib”的写法

#include <vector>

namespace Ayin {

	namespace {

#ifdef AYIN_PLATFORM_WINDOWS
		HIMC s_MainWindowImeContext = nullptr;	//编辑（文字输入）的IME配置
		bool s_MainWindowImeDisabled = false;	//编辑（文字输入）IME状态

		// 辅助方法
		HWND GetMainWindowHandle()
		{
			// GLFW 只暴露跨平台窗口指针；Win32 输入法/IME 操作需要原生 HWND。
			GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
			return glfwGetWin32Window(window);
		}
#endif

	}


	void Platform::DisableImeForMainWindow()
	{
#ifdef AYIN_PLATFORM_WINDOWS
		HWND nativeWindow = GetMainWindowHandle();
		if (!nativeWindow) {
			AYIN_CORE_WARN("Failed to disable IME: invalid native window");
			return;
		}

		if (s_MainWindowImeDisabled)
			return;

		//! IME——Input Method Editor，输入法编辑器。
		//! 简单说：
		//! - 键盘布局是物理 / 语言布局层面，比如 English(US)、Chinese(Simplified)。
		//! - IME 是中文 / 日文 / 韩文这种“你按键后不是直接输入字符，而是先组合候选词”的输入系统。
		//! - 中文输入法里你打 w a s d，它可能先变成拼音候选，这就是 IME 在工作。
		//! - 按 Shift 切到英文状态时，通常不是切换键盘布局，而是让 IME 关闭中文组合输入，按键直接变成 ASCII 字母。

		// ImmAssociateContext(hwnd, nullptr) 会把该窗口的输入法上下文解绑。
		// 返回值是解绑前的旧上下文，必须保存起来，之后文本框需要输入中文时再恢复。
		s_MainWindowImeContext = ImmAssociateContext(nativeWindow, nullptr);
		s_MainWindowImeDisabled = true;
#endif
	}


	void Platform::RestoreImeForMainWindow()
	{
#ifdef AYIN_PLATFORM_WINDOWS
		HWND nativeWindow = GetMainWindowHandle();
		if (!nativeWindow) {
			AYIN_CORE_WARN("Failed to restore IME: invalid native window");
			return;
		}

		if (!s_MainWindowImeDisabled)
			return;

		// 把禁用时保存下来的输入法上下文重新关联回窗口，让 ImGui::InputText 这类控件可以正常输入中文（也不一定是中文，完全看原本的情况了）。
		ImmAssociateContext(nativeWindow, s_MainWindowImeContext);
		s_MainWindowImeDisabled = false;
#endif
	}


	void Platform::SetInputMethodToEnglish()
	{
#ifdef AYIN_PLATFORM_WINDOWS
		DisableImeForMainWindow();

		//x 下面是旧方案：取得当前窗口的输入法上下文，并关闭 IME 开启状态。
		//x 一些现代中文输入法会在窗口获得焦点后重新打开该状态，所以不够稳定。
		//x // 取得当前窗口的输入法上下文。输入法上下文保存了这个窗口上的 IME 状态。
		//x HIMC inputContext = ImmGetContext(nativeWindow);
		////x	- 找到这个窗口当前关联的 HIMC。
		////x	- 把这个 HIMC 交给调用者使用。
		////x	- 内部可能增加引用计数、锁定状态、登记一次访问，具体实现由 Windows / 输入法管理器决定。
		////x	- 因此文档要求用完必须 ImmReleaseContext(hwnd, himc)。
		//x if (!inputContext) {
		//x 	AYIN_CORE_WARN("Failed to switch IME to English mode: invalid IME context");
		//x 	return;
		//x }
		//x // 关闭 IME 开启状态。对中文输入法来说，这通常等价于切到英文输入模式，而不是切换键盘布局。
		//x ImmSetOpenStatus(inputContext, FALSE);
		//x // 使用完输入法上下文后必须释放，和 Win32 的 Get/Release 风格 API 保持配对。
		//x ImmReleaseContext(nativeWindow, inputContext);
		////x	- 归还访问权。
		////x	- 允许系统 / IME 正常管理这个 context 生命周期。
		////x	- 避免引用计数或内部资源泄漏。
		////x	- 避免长期持有导致状态不同步。
		////x	- 避免其他窗口 / 线程 / 输入法组件等待或看到异常状态。
		// 
		////! 关于win32API中的这些get和release风格接口，用占用来理解更容易，占用和不再占用。不是销毁资源，而是让系统知道这次访问结束了。
		////! 这是一种显式流程：谁借用，谁归还。Win32 API 很多都是这个风格，资源生命周期由调用者显式管理。很多资源并不是简单“对象拥有权”，而是“临时访问权/借用权”。
		////！并且这是C库，不支持RAII，所以需要我们显式发起“析构流程”，也就是Release，单靠栈帧式不够的。

		//x 下面是旧方案：切换 Windows 键盘布局到 English (United States)。
		//x 它会改变输入语言/键盘布局，可能影响整个系统或在应用关闭后不自动恢复，所以不适合这里只想“中文输入法切英文状态”的需求。
		//x // 00000409 是 Windows 中 English (United States) 的键盘布局 ID。
		//x // KLF_ACTIVATE 会在加载成功后尝试把该布局设为当前线程的活动布局。
		//x HKL englishLayout = LoadKeyboardLayoutW(L"00000409", KLF_ACTIVATE);
		//x if (!englishLayout) {
		//x 	AYIN_CORE_WARN("Failed to switch input method to English");
		//x 	return;
		//x }
		//x // 先切换当前线程布局，覆盖本线程创建窗口后的默认输入状态。
		//x ActivateKeyboardLayout(englishLayout, 0);
		//x // 通知主窗口请求输入语言切换，确保窗口焦点进入后也使用英文布局。
		//x SendMessageW(nativeWindow, WM_INPUTLANGCHANGEREQUEST, 0, reinterpret_cast<LPARAM>(englishLayout));
#endif
	}



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

	std::optional<std::string> FileDialogs::OpenFile(std::initializer_list<FileFilter> filters, const char* defaultPath)
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
				return std::nullopt;
			default:
				AYIN_CORE_ERROR("File open error: {0}", NFD_GetError());
				return std::nullopt;
		}

	}

	std::optional<std::string> FileDialogs::SaveFile(std::initializer_list<FileFilter> filters, const char* defaultName)
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
				return std::nullopt;
			default:
				AYIN_CORE_ERROR("File save error: {0}", NFD_GetError());
				return std::nullopt;
		}

	}

}
