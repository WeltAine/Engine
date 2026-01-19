#pragma once

#include "AyinPch.h" 

#include "Core.h"
#include "Ayin/Events/Event.h"


namespace Ayin {

	//窗口参数结构体，名称，长，宽
	struct WindowProps {
		std::string Title;
		unsigned int Width, Height;

		WindowProps(const std::string& title = "Ayin Engine", unsigned int width = 1280, unsigned int height = 720)
			:Title(title), Width(width), Height(height)
		{}

	};


	//抽象窗口基类，用于窗口的抽象层
	class AYIN_API Window {

	public:
		using EventCallbackFn = std::function<void(Event&)>; //？？？这个别名也受域影响么，我记得是的

		//全局创建方法
		static Window* Create(const WindowProps& props = WindowProps());//？？？默认参数将亡值了，晚点查查

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		//设置观察者（回调），该观察者用于Event分发
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;

		//垂直同步
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;


		virtual ~Window() {};
	};


}
//有一种除了基础类型外，其余统统const &的习惯，不过他确实这么推荐过