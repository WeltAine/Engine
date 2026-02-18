#pragma once


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
	#define AYIN_ENABLE_ASSERT
#endif

//断言宏（宏里套宏，我也成为了自己讨厌的样子，不过这至少是正经封装。微软！变量类型也分平台么？啊！回答我！）
#ifdef AYIN_ENABLE_ASSERT
	#define AYIN_ASSERT(condition, ...) if(!(condition)) { AYIN_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }
	#define AYIN_CORE_ASSERT(condition, ...) if(!(condition)) { AYIN_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }
#else
	#define AYIN_ASSERT(condition, ...)
	#define AYIN_CORE_ASSERT(condition, ...)
#endif 

