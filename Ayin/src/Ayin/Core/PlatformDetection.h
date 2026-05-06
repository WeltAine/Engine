#pragma once

// 平台检测（检测项目被编译到哪一个平台上）
// Windows平台
#ifdef _WIN32										//Windows上无论x86还是x64都有该宏
#ifdef _WIN64
#define AYIN_PLATFORM_WINDOWS_64			//正在生成的 exe 文件，是专门给 64 位 Windows 系统运行的程序。
#else
#error "X86 Builds are not supported!"
#endif
//	苹果平台
#elif defind(__APPLE__) || define(__MACH__)
#include<TargetConditionals.h>
//! 苹果任何一种类型的的设备平台都会定义TARGET_OS_MAC
//! 所以我们将其放到最后检测
#if TARGET_IPHONE_SIMULATOR == 1
#error "IOS simulator is not supported!"
#elif TARGET_OS_IPHONE == 1
#error "IOS is not supported!"
#elif TARGET_OS_MAC == 1
#error "MacOS is not supported!"
#else 
#error "Unknown Apple platform!"
#endif
// 安卓平台
//! 安卓的内核是Linux，两者都会定义__linux__
//! 所以先检测安卓然后检测Linux
//? 我们真的要考虑用安卓开发游戏么
#elif defind(__ANDROID__)
#error "Android is not supported!"
// Linux平台
#elif defind(__linux__)
#error "Linux is not supported!"
#else
#error "Unknown platform!"
#endif
