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

	class AYIN_API Platform
	{

	public:

		//! 逻辑上一共就两个文字模式，输入文字时的文字模式，操作时的文字模式（为英文模式）
		//! 只在切换DiableIme和RestoreIme时才才更换文字模式。
		//! 而在切入DisableIme前一定是输入状态，具体使用的是中文还是英文取决于操作者的意图。该模式会被记录
		//! 切入RestoreIme时会恢复操作者在进行上一次输入时使用的文字模式。并非系统默认的文字模式（比如你上一次使用的时英文作为输入，而不是中文，那么下一次在其它位置输入时也是英文）

		// 禁用主窗口的 IME 上下文。用于编辑器场景操作区，避免 WASD/快捷键被中文输入法截获。
		// 关闭对话 IME
		static void DisableImeForMainWindow();

		// 恢复主窗口启动时保存的 IME 上下文。用于 ImGui 文本框需要中文输入时临时恢复输入法。
		// 恢复对话 IME
		static void RestoreImeForMainWindow();

		// 将当前应用窗口的 IME 切到英文输入状态。当前实现等价于禁用主窗口 IME 上下文。
		// 关闭对话 IME
		static void SetInputMethodToEnglish();

		//! 目前修改的是整个窗口的Ime，所以是全局性的
	};

}
