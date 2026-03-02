#pragma once

namespace Ayin {

	/// <summary>
	/// 图形上下文，创建与交换缓冲
	/// </summary>
	class GraphicsContext {
	public:

		virtual void Init() = 0;
		virtual void SwapBuffer() = 0;
	};

}