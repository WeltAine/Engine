#pragma once


#ifdef AYIN_PLATFORM_WINDOWS
	// 存在一个常见的 Windows/MSVC 与第三方库（glaze）冲突问题。
	// windows.h 头文件定义了 max 和 min 作为宏，而 Glaze 库中使用了 std::numeric_limits<size_t>::max()。
	// 当编译时，windows.h 的宏会干扰 std::max 的调用，导致编译错误。
	// error C4003: 类函数宏的调用"max"参数不足
	// error C2589 : "(" : "::"右边的非法标记
	// 这些错误都指向 buffer_traits.hpp 中的 std::numeric_limits<size_t>::max() 被误解析。
    #define WIN32_LEAN_AND_MEAN //排除不常用的 Windows API，减少头文件大小
    #define NOMINMAX			//阻止 Windows 头文件定义 max 和 min 宏
	#include <windows.h>
#endif

#include <iostream>
#include <fstream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <array>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "Ayin/Core/Log.h"

#include "Ayin/Debug/Instrumentor.h"

#include <entt/entt.hpp>
