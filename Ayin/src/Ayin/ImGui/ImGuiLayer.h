#pragma once

#include "Ayin/Core.h"
#include "Ayin/Layer.h"


#include "Ayin/Events/ApplicationEvent.h"
#include "Ayin/Events/KeyEvent.h"
#include "Ayin/Events/MouseEvent.h"

namespace Ayin {

	class AYIN_API ImGuiLayer : public Layer {

	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;//加入层栈
		virtual void OnDetach() override;//退出层栈
		virtual void OnImGuiRender() override;//ImGui渲染

		void Begin();
		void End();

	private:
		float m_Time = 0.0f;
	};

}

