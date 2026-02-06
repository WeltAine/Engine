#pragma once
#include "AyinPch.h"
#include "Ayin/Events/Event.h"
namespace Ayin {


	//基础层类，可以被添加到层栈中
	class Layer
	{
	public:

		Layer(const std::string& LayerName = "Layer")
			:m_DebugName(LayerName) 
		{}

		virtual ~Layer() {};

		virtual void OnAttach() {};//加入层栈
		virtual void OnDetach() {};//退出层栈
		virtual void OnUpdate() {};//层更新
		/// <summary>
		/// ImGui渲染（不会在Layer的OnUpdate中执行，OnUpdate中最多设置渲染信息，OnImGuiRender之后会被迁到渲染线程上）
		/// </summary>
		virtual void OnImGuiRender() {};
		virtual void OnEvent(Event& event) {};

		const std::string& GetName() const { return m_DebugName; };

	protected:
		std::string m_DebugName;//层名称
	};
}


