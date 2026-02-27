#pragma once

#include "Ayin/Core.h"
#include <memory>
#include <vector>

#include "Buffer.h"

namespace Ayin{

	class AYIN_API VertexArray 
	{
	// OpenGL中的顶点数组具备很强的代码山下文敏感性，特别是在其作用域范围上
	// 对于其方法我期望设计成非上下文敏感的，所以Add...与Set...方法其内部会恢复之前的上下文状态
	// 同时需要手动管理上下文的开启，也就是bind与Unbind
	// 之前的缓冲可能也会采取这个方法


	public:
		/// <summary>
		/// 创建顶线数组，是顶点缓冲，索引缓冲（工厂模式创建）
		/// </summary>
		/// <returns></returns>
		static VertexArray* Create();

		virtual ~VertexArray() = default;

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;

		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;

	};


}