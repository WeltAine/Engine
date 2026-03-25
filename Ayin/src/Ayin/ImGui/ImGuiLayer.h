#pragma once

#include "Ayin/Core.h"
#include "Ayin/Layer.h"


#include "Ayin/Events/ApplicationEvent.h"
#include "Ayin/Events/KeyEvent.h"
#include "Ayin/Events/MouseEvent.h"

namespace Ayin {

	/// <summary>
	/// ImGui逻辑层
	/// </summary>
	//! layer目前是我们结构中承载自定义逻辑的部分，而ImGuiLayer承载的则是UI相关逻辑（他需要处理事件分析布局之类的事情）
	//! 尽管我们在Cpp中使用了OnImGuiRender方法，但实际上也可以不写，因为目前其核心功能不是这个
	//! 而是保证每一帧中UI功能可以被正确触发
	class AYIN_API ImGuiLayer : public Layer {

	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;//加入层栈
		virtual void OnDetach() override;//退出层栈
		virtual void OnImGuiRender() override;//ImGui渲染

		void Begin();
		void End();

	private:
		float m_Time = 0.0f;
	};

}

