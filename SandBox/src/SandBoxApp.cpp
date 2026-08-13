#include <AyinPch.h>

#include <Ayin.h>

// ---入口点--------------------------
#include "Ayin/Core/EntryPoint.h"
// -----------------------------------

#include "TestLayer.h"


class SandBox : public Ayin::Application {

public:
	SandBox() {

		PushLayer(new TestLayer{});

	};

	~SandBox() {};

};



Ayin::Application* Ayin::CreatApplication() {

	return new SandBox();

}
