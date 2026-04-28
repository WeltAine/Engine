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

		inline static void ProcessEvent(Event& e) { s_Instance->OnEvent(e); };

		inline static void TransitionToNextFrame() { s_Instance->TransitionToNextFrameImpl(); }

		inline static bool IsKeyPressed(KeyCode keyCode) { return s_Instance->IsKeyPressedImpl(keyCode); };

		inline static bool GetMouseButton(MouseCode button) { return s_Instance->GetMouseButtonImpl(button); };
		inline static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); };
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); };
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); };
		inline static float GetScrollXoffset() { return s_Instance->GetScrollXoffsetImpl(); }
		inline static float GetScrollYoffset() { return s_Instance->GetScrollYoffsetImpl(); }
		inline static float GetScrollX() { return s_Instance->GetScrollX(); };
		inline static float GetScrollY() { return s_Instance->GetScrollY(); };


	protected:
		//实现侧实现下面这些接口
		virtual void OnEvent(Event& e) {};
		virtual void TransitionToNextFrameImpl() {};

		virtual bool IsKeyPressedImpl(KeyCode keyCode) const = 0;

		virtual bool GetMouseButtonImpl(MouseCode button) const = 0;
		virtual std::pair<float, float> GetMousePositionImpl() const = 0;
		virtual float GetMouseXImpl() const = 0;
		virtual float GetMouseYImpl() const = 0;
		virtual float GetScrollYoffsetImpl() const = 0;
		virtual float GetScrollXoffsetImpl() const = 0;
		virtual float GetScrollXImpl() const = 0;
		virtual float GetScrollYImpl() const = 0;

	private:
		static Scope<Input> Create();

	protected:
		static Scope<Input> s_Instance;//单例的保证（本身是抽象类无法直接构造实例），同时实现侧的指针
	};

}
