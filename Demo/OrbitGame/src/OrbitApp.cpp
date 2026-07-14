#include <AyinPch.h>

#include <Ayin.h>

#include "OrbitLayer.h"

#include "Ayin/Core/EntryPoint.h"

class OrbitApplication : public Ayin::Application {

public:
	OrbitApplication() {
		PushLayer(new OrbitLayer{});
	}

	~OrbitApplication() override = default;

};

Ayin::Application* Ayin::CreatApplication() {

	return new OrbitApplication();

}
