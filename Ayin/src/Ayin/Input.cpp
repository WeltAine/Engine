#include <AyinPch.h>

#include "Ayin/Input.h"

#include "Platform/Windows/WindowsInput.h"

namespace Ayin {


	Scope<Input> Input::s_Instance = CreateScope<WindowsInput>();
	//因为Input可以说就是一个方法集合，目前没有任何独属于类的信息，自然也不需要什么构造方法，我们可以直接这样定义单例实例


}