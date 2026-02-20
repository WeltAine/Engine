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
		using EventCallbackFn = std::function<void(Event&)>;

		/// <summary>
		/// 创建窗口（抽象侧）
		/// </summary>
		/// <param name="props">窗口参数（有默认值）</param>
		/// <returns>抽象窗口指针</returns>
		static Window* Create(const WindowProps& props = WindowProps());
		//必须要用const，参数赋值是一个{}外阶段，根据C++的生命周期控制，这里的WindowProps()构造语句结束之后该对象消亡，也就是在进入{}之前会消亡，所以是一个将亡值

		/// <summary>
		/// 处理窗口的系统事件，通过图形上下文接口交换帧缓冲
		/// </summary>
		virtual void OnUpdate() = 0;

		virtual void* GetNativeWindow() const = 0;//获取实现侧窗口指针（因为是void*，这个方法应该只对实现侧来说有意义）

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