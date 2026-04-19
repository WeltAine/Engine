#include <AyinPch.h>

#include <Ayin.h>

// ---入口点--------------------------
#include "Ayin/Core/EntryPoint.h"
// -----------------------------------

#include "SandBox2D.h"
//#include "ExampleLayer.h"


class SandBox : public Ayin::Application {

public:
	SandBox() {

		//PushLayer(new ExampleLayer{});//! 这里我们对于层的概念更加清晰了一些——游戏循环中某一模块的逻辑
		PushLayer(new SandBox2D{});

	};

	~SandBox() {};

};



Ayin::Application* Ayin::CreatApplication() {

	return new SandBox();

}