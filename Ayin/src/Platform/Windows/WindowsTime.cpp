#include <AyinPch.h>

#include "Platform/Windows/WindowsTime.h"

#include "Ayin/Core/Log.h"

#include <GLFW/glfw3.h>

namespace Ayin {


	void WindowsTime::OnUpdateImpl() {

		// 时间更新
		Timestep runTime = (float)glfwGetTime();//已经运行的时间长度
		m_FrameInterval = runTime - m_RunTime;
		m_RunTime = runTime;
		//? 这个过程我觉得应该单独设置一个Time外观来完成，甚至是单独弄一个为其弄一个计时层和ImGui一样，一起更新
		//? glfwGetTime这类底层API应该封在一个Platform中，像Input和WindowsInput那样
		AYIN_CORE_INFO("FrameInterval:{0}s  ({1}ms)", float(m_FrameInterval), m_FrameInterval.GetMilliseconds());

	}


}