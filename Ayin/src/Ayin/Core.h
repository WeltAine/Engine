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

