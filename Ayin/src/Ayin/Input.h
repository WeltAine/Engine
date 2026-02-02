#pragma once

#include <AyinPch.h>
#include "Ayin/Core.h"

namespace Ayin {


	//轮询Input接口（抽象侧），目前设计为一个全局单例（Cherno曾有一个很精辟的总结，单例其实就是一些里方法的集合，当然这不意味着所有方法都是static）
	//意在用于访问交互状态，实现更复杂的互动，比如特别的快捷键
	class AYIN_API Input {

	public:
		inline static bool IsKeyPressed(int keyCode) { return s_Instance->IsKeyPressedImpl(keyCode); };

		inline static bool GetMouseButton(int button) { return s_Instance->GetMouseButtonImpl(button); };
		inline static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); };
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); };
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); };

	protected:
		//实现侧实现下面这些接口
		virtual bool IsKeyPressedImpl(int keyCode) const = 0;

		virtual bool GetMouseButtonImpl(int button) const = 0;
		virtual std::pair<float, float> GetMousePositionImpl() const = 0;
		virtual float GetMouseXImpl() const = 0;
		virtual float GetMouseYImpl() const = 0;


	private:
		static Input* s_Instance;//单例的保证（本身是抽象类无法直接构造实例），同时实现侧的指针
	};

}
