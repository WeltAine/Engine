#include <AyinPch.h>

#include "Ayin/Core/Window.h"

#if AYIN_PLATFORM_WINDOWS
	#include "Platform/Windows/WindowsWindow.h"
#endif


namespace Ayin {


	Window* Window::Create(const WindowProps& props) {

#if AYIN_PLATFORM_WINDOWS
		return  new WindowsWindow(props);
#endif
		AYIN_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
	}


}