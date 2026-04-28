#pragma once

#include "Ayin/Core/Input.h"
#include "Ayin/Events/Event.h"
#include "Ayin/Events/MouseEvent.h"
#include "Ayin/Events/KeyEvent.h"

namespace Ayin {

	class WindowsInput : public Input
	{

	protected:

		//x GLFW没有访问鼠标滚轮的轮询，我们得自己写, 接入到回调中
		bool OnScrollEvent(MouseSrolledEvent& e);

	protected:
		// 通过 Input 继承
		virtual void OnEvent(Event& e) override;
		virtual void TransitionToNextFrameImpl() override;

		virtual bool IsKeyPressedImpl(KeyCode keyCode) const override;

		virtual bool GetMouseButtonImpl(MouseCode button) const override;
		virtual std::pair<float, float> GetMousePositionImpl() const override;
		virtual float GetMouseXImpl() const override;
		virtual float GetMouseYImpl() const override;
		virtual float GetScrollXoffsetImpl() const override { return m_MouseScrollXoffset; };
		virtual float GetScrollYoffsetImpl() const override { return m_MouseScrollYoffset; };
		virtual float GetScrollXImpl() const override { return m_ScrollXAccumulator; };
		virtual float GetScrollYImpl() const override { return m_ScrollYAccumulator; };


	private:
		float m_MouseScrollXoffset = 0.0f, m_MouseScrollYoffset = 0.0f;				//鼠标滚轮
		float m_ScrollXAccumulator = 0.0f, m_ScrollYAccumulator = 0.0f;				//鼠标滚轮累计值


	};

}

