#include "AyinPch.h"

#include "Platform/Windows/WindowsInput.h"
#include "Ayin/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Ayin {

	float WindowsInput::s_MouseScrollXoffset = 0.0f;
	float WindowsInput::s_MouseScrollYoffset = 0.0f;				
	float WindowsInput::s_ScrollXAccumulator = 0.0f;
	float WindowsInput::s_ScrollYAccumulator = 0.0f;				




	void Input::OnEvent(Event& e) { WindowsInput::OnEventImpl(e); };

	void Input::TransitionToNextFrame() { WindowsInput::TransitionToNextFrameImpl(); };

	bool Input::IsKeyPressed(KeyCode keyCode) { return WindowsInput::IsKeyPressedImpl(keyCode); };

	bool Input::GetMouseButton(MouseCode button) { return WindowsInput::GetMouseButtonImpl(button); };
	std::pair<float, float> Input::GetMousePosition() { return WindowsInput::GetMousePositionImpl(); };
	float Input::GetMouseX() { return WindowsInput::GetMouseXImpl(); };
	float Input::GetMouseY() { return WindowsInput::GetMouseYImpl(); };
	float Input::GetScrollXoffset() { return WindowsInput::GetScrollXoffsetImpl(); };
	float Input::GetScrollYoffset() { return WindowsInput::GetScrollYoffsetImpl(); };
	float Input::GetScrollX() { return WindowsInput::GetScrollXImpl(); };
	float Input::GetScrollY() { return WindowsInput::GetScrollYImpl(); };





	void WindowsInput::OnEventImpl(Event& e) {
	
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<MouseSrolledEvent>(WindowsInput::OnScrollEvent);

	};
	void WindowsInput::TransitionToNextFrameImpl() {
	
		s_MouseScrollXoffset = 0.0f, s_MouseScrollYoffset = 0.0f;

	};



	bool WindowsInput::OnScrollEvent(MouseSrolledEvent& e) {

		s_MouseScrollXoffset = e.GetXoffset();
		s_MouseScrollYoffset = e.GetYoffset();

		s_ScrollXAccumulator += s_MouseScrollXoffset;
		s_ScrollYAccumulator += s_MouseScrollYoffset;

		return false;
	};



	bool WindowsInput::IsKeyPressedImpl(KeyCode keyCode)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		int state = glfwGetKey(window, static_cast<uint32_t>(keyCode));

		return ( state == GLFW_PRESS || state == GLFW_REPEAT );
	}

	bool WindowsInput::GetMouseButtonImpl(MouseCode button)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		
		int state = glfwGetMouseButton(window, static_cast<uint32_t>(button));

		return state == GLFW_PRESS;
	}

	std::pair<float, float> WindowsInput::GetMousePositionImpl()
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		double x, y;
		glfwGetCursorPos(window, &x, &y);

		return std::pair<float, float>{(float)x, (float)y};
	}

	float WindowsInput::GetMouseXImpl()
	{
		auto[x, y] = GetMousePositionImpl();

		return x;
	}

	float WindowsInput::GetMouseYImpl()
	{
		auto [x, y] = GetMousePositionImpl();

		return y;
	}


}

