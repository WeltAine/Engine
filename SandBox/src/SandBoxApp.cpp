#include <Ayin.h>
#include <ImGui/imgui.h>


class ExampleLayer : public Ayin::Layer {

public:
	ExampleLayer()
		:Ayin::Layer("Example") 
	{}


	virtual void OnAttach() {
		ImGui::SetCurrentContext(Ayin::ImGuiLayer::GetImGuiCurrentContext());
	}

	virtual void OnUpdate() override {
		//AYIN_INFO("ExampleLayer::Update");
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_TAB)) {
			AYIN_TRACE("Tab key is pressed!");
		}
	}

	virtual void OnEvent(Ayin::Event& event) override {

		AYIN_TRACE("{0}", event);//输出接收到的日志

		if (event.GetEventType() == Ayin::KeyPressedEvent::GetStaticEventType()) {
			AYIN_INFO("{0}", (char)((Ayin::KeyEvent&)event).GetKeyCode());
		}
	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Test");
		ImGui::Text("Hello World");
		ImGui::End();
	}


};


class SandBox : public Ayin::Application {

public:
	SandBox() {

		PushLayer(new ExampleLayer());
		//PushOverlay(new Ayin::ImGuiLayer());//覆盖层应该完全由核心控制
	};

	~SandBox() {};

};



Ayin::Application* Ayin::CreatApplication() {

	return new SandBox();

}