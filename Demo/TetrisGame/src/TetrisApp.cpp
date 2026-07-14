#include <AyinPch.h>

#include <Ayin.h>

#include "TetrisLayer.h"

#include "Ayin/Core/EntryPoint.h"

class TetrisApplication : public Ayin::Application {

public:
	TetrisApplication() {
		PushLayer(new TetrisLayer{});
	}

	~TetrisApplication() override = default;

};

Ayin::Application* Ayin::CreatApplication() {

	return new TetrisApplication();

}
