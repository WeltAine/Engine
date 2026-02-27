#pragma once

#include "Ayin/Core.h"

// 包含各种Buffer抽象，集中管理，减少文件数量

namespace Ayin{


	enum class AYIN_API ShaderDataType
	{
		// 未知类型
		None = 0,
		// 向量
		Float, Float2, Float3 ,Float4,
		Int, Int2, Int3, Int4,
		Bool,
		// 矩阵
		Mat3, Mat4
		// 未知类型
	};

	static size_t ShaderDataTypeSize(ShaderDataType type) {

		switch (type) {

			case(ShaderDataType::Float):	return 4;
			case(ShaderDataType::Float2):	return 4 * 2;
			case(ShaderDataType::Float3):	return 4 * 3;
			case(ShaderDataType::Float4):	return 4 * 4;
			case(ShaderDataType::Int):		return 4;
			case(ShaderDataType::Int2):		return 4 * 2;
			case(ShaderDataType::Int3):		return 4 * 3;
			case(ShaderDataType::Int4):		return 4 * 4;
			case(ShaderDataType::Bool):		return 1;
			case(ShaderDataType::Mat3):		return 4 * 3 * 3;
			case(ShaderDataType::Mat4):		return 4 * 4 * 4;

		}

		AYIN_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;

	}

	struct AYIN_API BufferElement
	{
	
		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			:Type(type), Name(name), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{}

		uint32_t GetComponentCount() const {

			switch (this->Type) {

				case(ShaderDataType::Float):	return 1;
				case(ShaderDataType::Float2):	return 2;
				case(ShaderDataType::Float3):	return 3;
				case(ShaderDataType::Float4):	return 4;
				case(ShaderDataType::Int):		return 1;
				case(ShaderDataType::Int2):		return 2;
				case(ShaderDataType::Int3):		return 3;
				case(ShaderDataType::Int4):		return 4;
				case(ShaderDataType::Bool):		return 1;
				case(ShaderDataType::Mat3):		return 3 * 3;
				case(ShaderDataType::Mat4):		return 4 * 4;

			}

			AYIN_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;

		}


		ShaderDataType Type;
		std::string Name;
		size_t Size;
		uint32_t Offset;
		bool Normalized;
	
	};

	class AYIN_API BufferLayout 
	{

	public:
		BufferLayout() = default;

		BufferLayout(const std::initializer_list<BufferElement>& elements) 
			:m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}
		// 关于initializer_list的使用
		// 简单来说initializer_list 就是列表初始化或者叫初始化列表的化身（文档上两者的格式都是{"初始化列表"}），列表初始化可以称呼为基于初始化列表的初始化行为
		// 我们知道隐式转换序列中我们只允许进行一次用户定义转换
		// 但是initializer_list十分特殊，可以理解它是C++特性，可以以它为媒介发起一次无"转换计数"的转换，而且整个转换序列可以发起多次
		// BufferElement --> BufferLayout：消耗一次计数；
		// 如果以const std::initializer_list<BufferElement>& elements作为中间参数
		// BufferElement -(隧穿)-> BufferLayout：不消耗计数；就像量子隧穿是量子自身的特性，无代价转换就是initializer_list的特性（完整过程为BufferElement --> initializer_lise<BufferElement> -->  BufferLayout）
		// 对了，隐式转换关注的是从实参变为形参，所以最后的目标构造是不用算在其中的
		// 参考资料
		// https://en.cppreference.com/w/cpp/language/list_initialization.html
		// https://en.cppreference.com/w/cpp/language/initialization.html
		// https://en.cppreference.com/w/cpp/language/overload_resolution.html#Ranking_of_implicit_conversion_sequences
		// https://en.cppreference.com/w/cpp/utility/initializer_list/initializer_list.html


		//BufferLayout(const std::vector<BufferElement>& elements)
		//	:m_Elements(elements)
		//{
		//	CalculateOffsetsAndStride();
		//}


		void CalculateOffsetsAndStride() {

			size_t offset = 0;
			m_Stride = 0;

			for(auto& element : m_Elements) {
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}

		}

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); };
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); };
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); };
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); };

		inline size_t GetStride() const { return m_Stride; };

		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; };

	private:
		std::vector<BufferElement> m_Elements;
		size_t m_Stride;

	};

	/// <summary>
	/// 顶点缓冲抽象类
	/// </summary>
	class AYIN_API VertexBuffer 
	{

	public:

		/// <summary>
		/// 根据Renderer中的当前API来创建顶点缓冲，并且创建即绑定（工厂模式创建）
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

		/// <summary>
		/// 设置缓冲布局
		/// </summary>
		/// <param name="bufferLayout"> 布局参数 </param>
		virtual void SetLayout(const BufferLayout& bufferLayout) = 0;

		/// <summary>
		/// 获取缓冲布局
		/// </summary>
		/// <returns>缓冲布局</returns>
		virtual const BufferLayout& GetLayout() const = 0;
	};


	/// <summary>
	/// 索引缓冲抽象类
	/// </summary>
	class AYIN_API IndexBuffer 
	{

	public:

		/// <summary>
		/// 根据Renderer中的当前API来创建索引缓冲，并且创建即绑定（工厂模式创建）
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