#include <AyinPch.h>

#include <Ayin.h>

// ---入口点--------------------------
#include "Ayin/Core/EntryPoint.h"
// -----------------------------------

#include "EditorLayer.h"


class SandBox : public Ayin::Application {

public:
	SandBox() {

		PushLayer(new EditorLayer{});//! 这里我们对于层的概念更加清晰了一些——游戏循环中某一模块的逻辑

	};

	~SandBox() {};

};



Ayin::Application* Ayin::CreatApplication() {

	return new SandBox();

}