#include <Ayin.h>


class SandBox : public Ayin::Application {

public:
	SandBox() {};

	~SandBox() {};

};



//void main() {
//
//	SandBox* app = new SandBox();
//	app->Run();
//	delete app;
//
//}

Ayin::Application* Ayin::CreatApplication() {

	return new SandBox();

}