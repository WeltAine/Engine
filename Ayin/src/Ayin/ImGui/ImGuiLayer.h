#pragma once

#include "Ayin/Core.h"
#include "Ayin/Layer.h"

namespace Ayin {

	class AYIN_API ImGuiLayer : public Layer {

	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;//加入层栈
		virtual void OnDetach() override;//退出层栈
		virtual void OnUpdate() override;//层更新
		virtual void OnEvent(Event& event) override;


	private:
		float m_Time = 0.0f;
	};

}

