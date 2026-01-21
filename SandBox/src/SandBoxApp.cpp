#include <Ayin.h>


class ExampleLayer : public Ayin::Layer {

public:
	ExampleLayer()
		:Ayin::Layer("Example") 
	{}


	void OnUpdate() override {
		AYIN_INFO("ExampleLayer::Update");
	}

	void OnEvent(Ayin::Event& event) override {

		AYIN_TRACE("{0}", event);
	}

};


class SandBox : public Ayin::Application {

public:
	SandBox() {

		PushLayer(new ExampleLayer());
	};

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