#include "AyinPch.h"
#include "WindowsInput.h"
#include "Ayin/Application.h"

#include <GLFW/glfw3.h>

namespace Ayin {


	bool WindowsInput::IsKeyPressedImpl(int keyCode) const
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		int state = glfwGetKey(window, keyCode);

		return ( state == GLFW_PRESS || state == GLFW_REPEAT );
	}

	bool WindowsInput::GetMouseButtonImpl(int button) const
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		
		int state = glfwGetMouseButton(window, button);

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

