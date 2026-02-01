#pragma once
#include "AyinPch.h"

#include "Ayin/Core.h"
//#include "spdlog/fmt/bundled/format.h"

//#include <functional>
//#include <string>


//以类（结构体）的形式构造事件信息
//那些想要观察事件的类中，会有一个针对特定事件的回调方法
//事件派发类会接受发出的事件，然后通过Dispath方法发到指定的观察者（回调），并且使用了类似与访问者模式的实现，为Event扩展了派发功能，并且一些列重载也被分散为不同类中的观察者（回调方法），而且我们是自己判断是否符合重载中的参数类型
//我想我们目前还缺少一个管理事件的类
namespace Ayin {

#define BIND_EVENT_FUN(funcName) std::bind(&funcName, this, std::placeholders::_1)//事件处理回调（不是事件发生回调OnEvent）的包装器

	//事件枚举
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,	//窗口事件
		KeyPressed, KeyReleased, Text,												//键盘事件
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseSrolled,		//鼠标事件
		AppTick, AppUpdate, AppRender,											//应用事件

	};

	//事件分类枚举（一个事件可能同属属于多个分类）
	//？？？发现一件事前，如果使用enum class的话，是不能对枚举元素直接使用|或者&，但是使用enum就可以
	//话说回来我可以在枚举类中自定义方法么，比如定义|
	enum class EventCategory {
		None = 0,
		EventCategoryApplication	= BIT(0),
		EventCategoryInput			= BIT(1),
		EventCategoryKeyboard		= BIT(2),
		EventCategoryMouse			= BIT(3),
		EventCategoryMouseButton	= BIT(4)

	};


//对抽象方法的基础通用实现
//静态类型是用于获取编译期类型
//GetEventType怎是获取运行时类型，保证即使因多态而向基类游移情况下也获得原本的类型
#define EVENT_CLASS_TYPE(Type)	static EventType GetStaticEventType() { return EventType::##Type; }\
									virtual EventType GetEventType() const override { return GetStaticEventType(); }\
									virtual const char* GetName() const override { return #Type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const { return category; }


	//基础抽象类
	//具体的子类中记录子类自己的事件数据
	class AYIN_API Event {

		friend class EventDispatcher;

	public:
		bool handled = false;//事件是否被处理过了，可以用于后续阻断在Layer中的传递

	public:
		virtual EventType GetEventType() const = 0;//事件类型，虚函数，保证在多态时也能获得正确的原本事件类型
		virtual const char* GetName() const = 0;//事件名称
		virtual int GetCategoryFlags() const = 0;//事件类别
		virtual std::string ToString() const { return GetName(); };

		inline bool IsInCatagory(EventCategory category) {
			return GetCategoryFlags() & (int)category;
		}

	private:
	};

	//约束
	template <typename T>
	concept EventDerived = std::is_base_of_v<Ayin::Event, T>;

	//事件派发类
	//用于将事件
	class EventDispatcher {//看来不需要导出，外部不会使用是么，那就先这样

		template<EventDerived T>
		using EventFunc = std::function<bool(T&)>;

	public:
		EventDispatcher(Event& event)
			:m_Event(event)
		{}

		/// <summary>
		/// 将所接受的到事件发送给指定观察者，并尝试通知观察者
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="func"></param>
		/// <returns></returns>
		template<typename T>
		bool Dispatch(EventFunc<T> func) {

			if (m_Event.GetEventType() == T::GetStaticEventType()) {//对比派发起所接受的是事件的原本类型（引用的底层是指针，所以当然能触发多态了）和观察者期望的类型是否一致

				m_Event.handled = func(static_cast<T&>(m_Event));//通知观察者（调用回调）
				return true;

			}


			return false;
		}


	private:
		Event& m_Event;//派发器接受到的事件
	};


	//事件输出
	inline std::ostream& operator<< (std::ostream& os, const Event& e) {
		return os << e.ToString();
	}

	//spdlog输出自定义类型所需
	template <EventDerived Event>
	struct fmt::formatter<Event> : fmt::formatter<std::string> {
		template <typename FormatContext>
		auto format(const Event& e, FormatContext& ctx) const {
			return fmt::format_to(ctx.out(), "{}", e.ToString());
		}
	};


}