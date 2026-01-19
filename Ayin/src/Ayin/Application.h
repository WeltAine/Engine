#pragma once
//应用类


#include "Core.h"
#include "Window.h"

namespace Ayin {

	class AYIN_API Application
	{
	public:
		Application();

		virtual ~Application();

		void Run();

	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};


	Application* CreatApplication();//由Ayin应用实现（即客户端侧）
	//这样声明，但用extern来指示外部寻找，就导致引入该头文件的客户端侧必须提供实现，否则报错
	//所以这相当于一个步骤模式中的步骤，不过不那么典型就是了，也可以完全不这么想，这样的看法确实有些多余

}


