#include <Ayin.h>


class ExampleLayer : public Ayin::Layer {

public:
	ExampleLayer()
		:Ayin::Layer("Example") 
	{}


	void OnUpdate() override {
		//AYIN_INFO("ExampleLayer::Update");
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_TAB)) {
			AYIN_TRACE("Tab key is pressed!");
		}
	}

	void OnEvent(Ayin::Event& event) override {

		AYIN_TRACE("{0}", event);//输出接收到的日志

		if (event.GetEventType() == Ayin::KeyPressedEvent::GetStaticEventType()) {
			AYIN_INFO("{0}", (char)((Ayin::KeyEvent&)event).GetKeyCode());
		}
	}

};


class SandBox : public Ayin::Application {

public:
	SandBox() {

		PushLayer(new ExampleLayer());
		PushOverlay(new Ayin::ImGuiLayer());
	};

	~SandBox() {};

};



Ayin::Application* Ayin::CreatApplication() {

	return new SandBox();

}