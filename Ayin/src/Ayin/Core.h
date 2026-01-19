#pragma once


//通过宏来完成 _declspec(dllexport/dllimport)的自动化过程
//（只需要引入头文件就可以完成自动化处理，对于核心侧需要确保引入Core.h，对于客户端侧本质也是如此，不过我们会引入Ayin.h来供客户端侧使用）

#ifdef AYIN_PLATFORM_WINDOWS
	#ifdef AYIN_BUILD_DLL
		#define AYIN_API __declspec(dllexport)
	#else
		#define AYIN_API __declspec(dllimport)
	#endif
#else
	#error AYIN ONLY SUPPORT WINDOWS PLATFORM
#endif


//位移运算（可用作掩码）
#define BIT(x) (1 << x) 


//断言宏（宏里套宏，我也成为了自己讨厌的样子，不过这至少是正经封装。微软！变量类型也分平台么？啊！回答我！）
#ifdef AYIN_ENABLE_ASSERT
	#define AYIN_ASSERT(condition, ...) if(!(condition)) { AYIN_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }
	#define AYIN_CORE_ASSERT(condition, ...) if(!(condition)) { AYIN_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }
#else
	#define AYIN_ASSERT(condition, ...)
	#define AYIN_CORE_ASSERT(condition, ...)
#endif 

