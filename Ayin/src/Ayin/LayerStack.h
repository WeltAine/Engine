#pragma once
#include "AyinPch.h"
#include "Ayin/Layer.h"


namespace Ayin {

	//层栈类，以数组的方式存储层
	//层分两类，非覆盖层和覆盖层，后者位于层队列的末尾

	//目前有一些内存安全问题（在Pop时，指针会丢失，空间无法消除），好在Application目前所暴露的接口仅有Push相关，暂时无碍。
	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);//增加非覆盖层
		void PushOverlay(Layer* overlayer);//增加覆盖层，覆盖层位于栈的末尾是最后更新的（可以使它渲染在最上层，gizmos就是很好的例子）

		//移除方法在移除前会做检测，确保移除的是非覆盖层或覆盖层
		void PopLayer(Layer* layer);//移除非覆盖层
		void PopOverlay(Layer* overLayer);//移除覆盖层

		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); };//！！！不要加const，否则会i返回const_iterator
		std::vector<Layer*>::iterator end() { return m_Layers.end(); };//！！！不要加const，否则会i返回const_iterator


	private:
		//？？？不用指针，cherno解释是跟什么所有权相关，晚点再看一遍
		std::vector<Layer*> m_Layers;//层“栈”，因为我们的更新和事件在层间的走向是不同的，所以不会使用真正的栈
		unsigned int m_LayerInsertIndex = 0;

	};

}


