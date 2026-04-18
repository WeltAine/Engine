#include "AyinPch.h"

#include "Platform/Windows/WindowsInput.h"
#include "Ayin/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Ayin {


	bool WindowsInput::IsKeyPressedImpl(KeyCode keyCode) const
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		int state = glfwGetKey(window, static_cast<uint32_t>(keyCode));

		return ( state == GLFW_PRESS || state == GLFW_REPEAT );
	}

	bool WindowsInput::GetMouseButtonImpl(MouseCode button) const
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		
		int state = glfwGetMouseButton(window, static_cast<uint32_t>(button));

		return state == GLFW_PRESS;
	}

	std::pair<float, float> WindowsInput::GetMousePositionImpl() const
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		double x, y;
		glfwGetCursorPos(window, &x, &y);

		return std::pair<float, float>{(float)x, (float)y};
	}

	float WindowsInput::GetMouseXImpl() const
	{
		auto[x, y] = GetMousePositionImpl();

		return x;
	}

	float WindowsInput::GetMouseYImpl() const
	{
		auto [x, y] = GetMousePositionImpl();

		return y;
	}
}

