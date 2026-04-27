#include <AyinPch.h>

#include "Ayin/Core/Time.h"

#include "Platform/Windows/WindowsTime.h"

namespace Ayin {

	Scope<Time> Time::s_Instance = Time::Create();

	Scope<Time> Time::Create() {

#if AYIN_PLATFORM_WINDOWS
		return CreateScope<WindowsTime>();
#endif
		AYIN_CORE_ASSERT(false, "Unknown platform!");

	}

}