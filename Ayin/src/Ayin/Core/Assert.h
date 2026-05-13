#pragma once

#include <cstring>
#include <filesystem>


#ifdef AYIN_ENABLE_ASSERT


	namespace Ayin::Assert {

		//找到文件名称
		constexpr const char* CurrentFillName(const char* path) {

			//定位单字符出翔的最后一个位置，没有返回nullptr
			//const char* lastSlash = ::std::strrchr(path, '/');
			//const char* lastBackslash = ::std::strrchr(path, '\\');
			//这两方法不是constexpr

			const char* fileName = nullptr;

			while (*path) {
				if (*path == '/' || *path == '\\') {
					fileName = ++path;
				}
				else
					path++;
			}

			return fileName;

		}

	}

#define AYIN_ASSERT_NO_MSG(type, condition) \
	if(!(condition)){ \
		AYIN##type##ERROR("Assertion '{0}' failed at {1}:{2}", #condition,std::filesystem::path{__FILE__}, __LINE__); \
		AYIN_DEBUGBREAK(); \
	}

#define AYIN_ASSERT_WITH_MSG(type, condition, msg, ...) \
	if(!(condition)){ \
		AYIN##type##ERROR("Assertion Failed: " msg __VA_OPT__(,)__VA_ARGS__); \
		AYIN_DEBUGBREAK(); \
	}

	// 提取宏
#define AYIN_GET_MACRO(macro, ...) macro

#define AYIN_ASSERT(condition, ...) \
	AYIN_GET_MACRO(__VA_OPT__(AYIN_ASSERT_WITH_MSG,) AYIN_ASSERT_NO_MSG, __VA_ARGS__)(_, condition, __VA_ARGS__)

#define AYIN_CORE_ASSERT(condition, ...) \
	AYIN_GET_MACRO(__VA_OPT__(AYIN_ASSERT_WITH_MSG, ) AYIN_ASSERT_NO_MSG, __VA_ARGS__)(_CORE_, condition, __VA_ARGS__)

#else

#define AYIN_ASSERT(condition, ...)

#define AYIN_CORE_ASSERT(condition, ...)

#endif
