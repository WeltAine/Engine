#pragma once

#include "Ayin/Core.h"
#include "Event.h"

#include <sstream>


namespace Ayin{

	//鼠标按钮事件抽象基类
	class AYIN_API MouseButtonEvent : public Event {

	public:
		EVENT_CLASS_CATEGORY((int)EventCategory::EventCategoryInput|(int)EventCategory::EventCategoryMouse|(int)EventCategory::EventCategoryMouseButton);

		inline int GetMouseButton() const { return m_Button; };


	protected:
		MouseButtonEvent(int button)
			:m_Button(button)
		{}

		int m_Button;

	};


	class AYIN_API MouseButtonPressedEvent : public MouseButtonEvent {

	public:
		MouseButtonPressedEvent(int button)
			:MouseButtonEvent(button)
		{}

		EVENT_CLASS_TYPE(MouseButtonPressed);

		std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseButtonPreasedEvent: " << m_Button;
			return ss.str();
		}

	};

	class AYIN_API MouseButtonReleasedEvent : public MouseButtonEvent {

	public:
		MouseButtonReleasedEvent(int button)
			:MouseButtonEvent(button)
		{
		}

		EVENT_CLASS_TYPE(MouseButtonReleased);

		std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

	};



	class AYIN_API MouseMovedEvent : public Event {

	public:
		MouseMovedEvent(float mouseX, float mouseY)
			:Event(), m_MouseX(mouseX), m_MouseY(mouseY)
		{}

		EVENT_CLASS_TYPE(MouseMoved);
		EVENT_CLASS_CATEGORY((int)EventCategory::EventCategoryInput | (int)EventCategory::EventCategoryMouse);

		inline float GetMouseX() const { return m_MouseX; };
		inline float GetMouseY() const { return m_MouseY; };


		std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseMovedEvent: " << "(" << m_MouseX << ", " << m_MouseY << ")";
			return ss.str();
		}

	private:
		float m_MouseX, m_MouseY;
	};

	class AYIN_API MouseSrolledEvent : public Event {

	public:
		MouseSrolledEvent(float xoffset, float yoffset)
			:Event(), m_Xoffset(xoffset), m_Yoffset(yoffset)
		{
		}

		EVENT_CLASS_TYPE(MouseSrolled);
		EVENT_CLASS_CATEGORY((int)EventCategory::EventCategoryInput | (int)EventCategory::EventCategoryMouse);

		inline float GetXoffset() const { return m_Xoffset; };
		inline float GetYoffset() const { return m_Yoffset; };


		std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseSrolledEvent: " << "(" << m_Xoffset << ", " << m_Yoffset << ")";
			return ss.str();
		}

	private:
		float m_Xoffset, m_Yoffset;
	};


}