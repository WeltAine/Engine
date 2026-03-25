#pragma once

namespace Ayin {

	/// <summary>
	/// 图形上下文（决定用什么驱动绘制，是程序能够在窗口上绘制的关键）创建与交换缓冲
	/// </summary>
	class GraphicsContext {
	public:

		virtual void Init() = 0;
		virtual void SwapBuffer() = 0;
	};

}