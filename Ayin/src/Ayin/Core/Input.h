#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Events/Event.h"
#include "Ayin/Core/KeyCodes.h"
#include "Ayin/Core/MouseButtonCodes.h"

namespace Ayin {


	//轮询Input接口（抽象侧），目前设计为一个全局单例（Cherno曾有一个很精辟的总结，单例其实就是一些里方法的集合，当然这不意味着所有方法都是static）
	//意在用于访问交互状态，实现更复杂的互动，比如特别的快捷键
	//! 单例加外观
	class AYIN_API Input {

	protected:
		Input() = default;

	public:
		Input(const Input&) = delete;
		Input& operator=(const Input&) = delete;

	public:
		virtual ~Input() = default;

		static void OnEvent(Event& e);

		static void TransitionToNextFrame();

		static bool IsKeyPressed(KeyCode keyCode);

		static bool GetMouseButton(MouseCode button);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
		static float GetScrollXoffset();
		static float GetScrollYoffset();
		static float GetScrollX();
		static float GetScrollY();


	};

}
