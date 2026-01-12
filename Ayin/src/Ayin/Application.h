#pragma once
//应用类


#include "Core.h"

namespace Ayin {

	class AYIN_API Application
	{

	public:
		Application();

		virtual ~Application();

		void Run();

	};


	Application* CreatApplication();//由Ayin应用实现（即客户端侧）
	//这样声明，但用extern来指示外部寻找，就导致引入该头文件的客户端侧必须提供实现，否则报错

}


