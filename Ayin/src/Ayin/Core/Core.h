#pragma once
#include <memory>


//通过宏来完成 _declspec(dllexport/dllimport)的自动化过程
//（只需要引入头文件就可以完成自动化处理，对于核心侧需要确保引入Core.h，对于客户端侧本质也是如此，不过我们会引入Ayin.h来供客户端侧使用）

#ifdef AYIN_PLATFORM_WINDOWS
	#if AYIN_DENAMIC_LINK							//是否使用动态链接，决定AYIN_API的宏会扩展成什么样子
		#ifdef AYIN_BUILD_DLL						//当前位于核心侧还是客户端侧
			#define AYIN_API __declspec(dllexport)	//核心侧用于导出方法
		#else
			#define AYIN_API __declspec(dllimport)	//客户端侧用于使用导出的方法
		#endif
	#else 
		#define AYIN_API							//静态链接什么都不用做
	#endif
#else
	#error AYIN ONLY SUPPORT WINDOWS PLATFORM
#endif
// 在默认情况下，C++ DLL 内部的函数处于“隐藏”状态。操作系统无法识别哪些函数是可以被 EXE 程序调用的。
// 20 世纪 80 到 90 年代：开发人员必须手动编写 .def 文件（模块定义文件），逐一列出所有需要导出的函数。这种方式不仅繁琐，而且极易出错。
// 现代做法：微软引入了 __declspec(dllexport) 和 __declspec(dllimport) 。通过这种方式，可以直接在代码中对类或函数进行“标记”，声明：“嘿，让所有使用这个库的人都能看到它！”


//位移运算（可用作掩码）
#define BIT(x) (1 << x) 


#ifdef AYIN_DEBUG
	#if defined(AYIN_PLATFORM_WINDOWS)
		#define AYIN_DEBUGBREAK() __debugbreak()
	#elif
		#include <signal.h>
		#define AYIN_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
		
	#define AYIN_ENABLE_ASSERT
#endif


namespace Ayin {

	//简易资产管理系统（其实就是自己的引用）
	//TODO: 待完善
	//! 收束整合，方便之后修改
	//! 考虑到之后的渲染线程分离以及延迟渲染的实现，需要渲染线程对资源强引用，以避免在渲染时丢失资源

	template<typename T>
	using Ref = std::shared_ptr<T>;
	//对make_shared的封装
	template<typename T, typename... Args>
	constexpr Ref<T> CreateRef(Args&&... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	};

	template<typename T>
	using Scope = std::unique_ptr<T>;
	//对make_unique的封装
	template<typename T, typename... Args>
	constexpr Scope<T> CreateScope(Args&&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	};

	//ToDo 这里有个问题，高层很多都是抽象的，导致无法构造，构造方式不统一的情况下，可能没法调用
}

#define AYIN_CONCAT_IMPL(a, b) a##b
#define AYIN_CONCAT(a, b) AYIN_CONCAT_IMPL(a, b)

#include "Ayin/Core/Log.h"
#include "Ayin/Core/Assert.h"
