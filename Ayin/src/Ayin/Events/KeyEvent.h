#pragma once

#include "AyinPch.h"
#include "Event.h"

//#include <sstream>

namespace Ayin {
	
	//按键事件的抽象基类
	//记录了被按下的按键
	class AYIN_API KeyEvent : public Event {

	public:

		EVENT_CLASS_CATEGORY((int)EventCategory::EventCategoryKeyboard | (int)EventCategory::EventCategoryInput);

		inline int GetKeyCode() const { return m_KeyCode; };
		inline int GetScanCode() const { return m_ScanCode; };


	protected:

		KeyEvent(int keyCode, int scanCode)
			:Event(), m_KeyCode(keyCode), m_ScanCode(scanCode)
		{}

		int m_KeyCode;
		int m_ScanCode;

	};

	
	class AYIN_API KeyPressedEvent : public KeyEvent {

	public:

		KeyPressedEvent(int keyCode, int scanCode, int repeatCount)
			:KeyEvent(keyCode, scanCode), m_RepeatCount(repeatCount)
		{}

		EVENT_CLASS_TYPE(KeyPressed);

		inline int GetRepeatCount() const { return m_RepeatCount; };

		std::string ToString() const override{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << "(" << m_RepeatCount << "repeats)";
			return ss.str();
		}

	private:

		int m_RepeatCount;//按钮重复次数（对于那些按住方向键，然后快速滚动列表的功能）
	};


	class AYIN_API KeyReleasedEvent : public KeyEvent {

	public:
		KeyReleasedEvent(int keyCode, int scanCode)
			:KeyEvent(keyCode, scanCode)
		{}

		EVENT_CLASS_TYPE(KeyReleased);

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

	private:

	};


	class AYIN_API TextEvent : public KeyEvent 
	{

	public:
		TextEvent(int keyCode, int scanCode)
			:KeyEvent(keyCode, scanCode)
		{}

		EVENT_CLASS_TYPE(Text);

		std::string ToString() const override {
			std::stringstream ss;
			ss << "TextEvent: " << m_KeyCode;
			return ss.str();
		}

	};
}