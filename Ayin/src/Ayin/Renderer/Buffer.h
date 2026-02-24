#pragma once

#include "Ayin/Core.h"

// 包含各种Buffer抽象，集中管理，减少文件数量

namespace Ayin{

	/// <summary>
	/// 顶点缓冲抽象类
	/// </summary>
	class AYIN_API VertexBuffer {

	public:

		/// <summary>
		/// 根据Renderer中的当前API来创建顶点缓冲
		/// </summary>
		/// <param name="vertices">顶点缓冲数据</param>
		/// <param name="size">数据字节大小</param>
		/// <returns></returns>
		static VertexBuffer* Create(float* vertices, size_t size);

		virtual ~VertexBuffer() = default;

		/// <summary>
		/// 绑定顶点缓冲到当前上下文
		/// </summary>
		virtual void Bind() const = 0;

		/// <summary>
		/// 从当前上下文中解除绑定该顶点缓冲
		/// </summary>
		virtual void UnBind() const = 0;
	};


	/// <summary>
	/// 索引缓冲抽象类
	/// </summary>
	class AYIN_API IndexBuffer {

	public:

		/// <summary>
		/// 根据Renderer中的当前API来创建索引缓冲
		/// </summary>
		/// <param name="vertices">索引缓冲数据</param>
		/// <param name="size">缓冲中的int数量</param>
		/// <returns></returns>
		static IndexBuffer* Create(uint32_t* indices, uint32_t count);

		virtual ~IndexBuffer() = default;

		/// <summary>
		/// 绑定索引缓冲到当前上下文
		/// </summary>
		virtual void Bind() const = 0;

		/// <summary>
		/// 从当前上下文中解除绑定该索引缓冲
		/// </summary>
		virtual void UnBind() const = 0;

		/// <summary>
		/// 获取索引缓冲中元素数量
		/// </summary>
		/// <returns></returns>
		virtual uint32_t GetCount() const = 0;
	};

}