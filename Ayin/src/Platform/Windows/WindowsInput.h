#pragma once

#include "Ayin/Core/Input.h"
#include "Ayin/Events/Event.h"
#include "Ayin/Events/MouseEvent.h"
#include "Ayin/Events/KeyEvent.h"

namespace Ayin {

	class WindowsInput
	{
		friend class Input;

	protected:

		//x GLFW没有访问鼠标滚轮的轮询，我们得自己写, 接入到回调中
		static bool OnScrollEvent(MouseSrolledEvent& e);


		// 通过 Input 继承
		static void OnEventImpl(Event& e);
		static void TransitionToNextFrameImpl();
	
		static bool IsKeyPressedImpl(KeyCode keyCode);

		static bool GetMouseButtonImpl(MouseCode button);
		static glm::vec2 GetMousePositionImpl();
		static float GetMouseXImpl();
		static float GetMouseYImpl();
		static float GetScrollXoffsetImpl() { return s_MouseScrollXoffset; };
		static float GetScrollYoffsetImpl() { return s_MouseScrollYoffset; };
		static float GetScrollXImpl() { return s_ScrollXAccumulator; };
		static float GetScrollYImpl() { return s_ScrollYAccumulator; };



	private:
		static float s_MouseScrollXoffset, s_MouseScrollYoffset;				//鼠标滚轮
		static float s_ScrollXAccumulator, s_ScrollYAccumulator;				//鼠标滚轮累计值


	};

}

