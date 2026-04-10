#include "AyinPch.h"
#include "LayerStack.h"


namespace Ayin {


	LayerStack::LayerStack() {
	}

	LayerStack::~LayerStack() {
		//cherno所说的所有权应该就是指当Stack析构时，Layer*该何去何从
		for (Layer* layer : m_Layers) {
			layer->OnDetach();
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

		auto iterator = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer);
		//? find方法在查找失败时返回的是查找范围中的最后一个迭代器！
		//! 如果有人该方法试图弹出覆盖层会导致m_Layers.begin() + m_LayerInsertIndex被find返回
		//!     普通层	   |	覆盖层
		//! [][][][被返回] | [想查找][][]
		//!        ^^^^^  

		if (iterator != m_Layers.begin() + m_LayerInsertIndex) {
			//？？？感觉有风险
			//我觉得应该改成shared_ptr,否则有泄露风险
			//(*(iterator._Ptr))->OnDetach();//访问这么复杂么？
			layer->OnDetach();
			m_Layers.erase(iterator);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* overLayer) {

		auto iterator = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), overLayer);

		if (iterator != m_Layers.end()) {
			//(*(iterator._Ptr))->OnDetach();
			overLayer->OnDetach();
			m_Layers.erase(iterator);
			// erase会触发元素析构，重新保证有效元素的空间连续；确保在析构前完成必要的处理
			// 顺带一提，移动范围内的迭代器都会失效，再次使用它们是未定义行为，可能崩溃
		}


	}

}