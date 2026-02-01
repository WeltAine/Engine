#pragma once

#include "Ayin/Core.h"
#include "Ayin/Layer.h"


#include "Ayin/Events/ApplicationEvent.h"
#include "Ayin/Events/KeyEvent.h"
#include "Ayin/Events/MouseEvent.h"

namespace Ayin {

	class AYIN_API ImGuiLayer : public Layer {

	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;//加入层栈
		virtual void OnDetach() override;//退出层栈
		virtual void OnUpdate() override;//层更新
		virtual void OnEvent(Event& event) override;

	private:
		bool OnKeyPressedEvent(KeyPressedEvent& event);
		bool OnKeyReleasedEvent(KeyReleasedEvent& event);
		bool OnTextEvent(TextEvent& event);

		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);
		bool OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& event);
		bool OnMouseMovedEvent(MouseMovedEvent& event);
		bool OnMouseSrolled(MouseSrolledEvent& event);

		bool OnWindowResizeEvent(WindowResizeEvent& event);


	private:
		float m_Time = 0.0f;
	};

}

