#include <AyinPch.h>

#include "Ayin/Core/Input.h"

#if AYIN_PLATFORM_WINDOWS
	#include "Platform/Windows/WindowsInput.h"
#endif


namespace Ayin {


	Scope<Input> Input::s_Instance = Input::Create();//! 类的静态成员初始化（即使写在 .cpp 文件中），依然属于类的作用域，因此可以自由访问类的私有静态成员函数 Create()
	//因为Input可以说就是一个方法集合，目前没有任何独属于类的信息，自然也不需要什么构造方法，我们可以直接这样定义单例实例


	Scope<Input> Input::Create() {

#if AYIN_PLATFORM_WINDOWS
		return CreateScope<WindowsInput>();
#endif
		AYIN_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
	}

}