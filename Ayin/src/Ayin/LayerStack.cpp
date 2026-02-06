#include "AyinPch.h"
#include "LayerStack.h"


namespace Ayin {


	LayerStack::LayerStack() {
	}

	LayerStack::~LayerStack() {
		//cherno所说的所有权应该就是指当Stack析构时，Layer*该何去何从
		for (Layer* layer : m_Layers) {
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer* layer) {

		 m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		 m_LayerInsertIndex++;
		 layer->OnAttach();

	}

	void LayerStack::PushOverlay(Layer* overLayer) {

		//？？？我想我们是不是要约束一下防止有多个覆盖层
		m_Layers.emplace_back(overLayer);
		overLayer->OnAttach();

	}

	void LayerStack::PopLayer(Layer* layer) {

		auto iterator = std::find(m_Layers.begin(), m_Layers.end(), layer);

		if (iterator != m_Layers.end()) {
			//？？？感觉有风险
			//我觉得应该改成shared_ptr,否则有泄露风险
			//(*(iterator._Ptr))->OnDetach();//访问这么复杂么？
			m_Layers.erase(iterator);
			layer->OnDetach();
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* overLayer) {

		auto iterator = std::find(m_Layers.begin(), m_Layers.end(), overLayer);

		if (iterator == m_Layers.end()) {
			//(*(iterator._Ptr))->OnDetach();
			m_Layers.erase(iterator);
			overLayer->OnDetach();
		}


	}

}