#pragma once

#include "Ayin/Core/Core.h"

// 包含各种Buffer抽象，集中管理，减少文件数量
//! 不难发现，所有含Create工厂方法的类，都象征着一种资源（Buffer，shader），它们在GPU中占据一定内存

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


	//////////////////////////////////////////////////////////////////////////////////////////
	/// VertexBuffer /////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	struct AYIN_API BufferElement
	{
	
		BufferElement(uint32_t locationIndex, ShaderDataType type, const std::string& name, bool normalized = false)
			:LocationIndex(locationIndex), Type(type), Name(name), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
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

		uint32_t LocationIndex;
		ShaderDataType Type;
		std::string Name;
		size_t Size;
		uint32_t Offset;
		bool Normalized;
	
	};

	/// <summary>
	/// VAO数据布局（BufferElement负责各自元素的信息），只负责步进
	/// </summary>
	class AYIN_API BufferLayout 
	{

	public:
		BufferLayout() = default;

		BufferLayout(const std::initializer_list<BufferElement>& elements) 
			:m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}
		//? 内容有误（之后修改）
		//x 关于initializer_list的使用
		//x 简单来说initializer_list 就是列表初始化或者叫初始化列表的化身（文档上两者的格式都是{"初始化列表"}），列表初始化可以称呼为基于初始化列表的初始化行为
		//x 我们知道隐式转换序列中我们只允许进行一次用户定义转换
		//x 但是initializer_list十分特殊，可以理解它是C++特性，可以以它为媒介发起一次无"转换计数"的转换，而且整个转换序列可以发起多次
		//x BufferElement --> BufferLayout：消耗一次计数；
		//x 如果以const std::initializer_list<BufferElement>& elements作为中间参数
		//x BufferElement -(隧穿)-> BufferLayout：不消耗计数；就像量子隧穿是量子自身的特性，无代价转换就是initializer_list的特性（完整过程为BufferElement --> initializer_lise<BufferElement> -->  BufferLayout）
		//x 对了，隐式转换关注的是从实参变为形参，所以最后的目标构造是不用算在其中的
		// 参考资料
		// https://en.cppreference.com/w/cpp/language/list_initialization.html
		// https://en.cppreference.com/w/cpp/language/initialization.html
		// https://en.cppreference.com/w/cpp/language/overload_resolution.html#Ranking_of_implicit_conversion_sequences
		// https://en.cppreference.com/w/cpp/utility/initializer_list/initializer_list.html


		void CalculateOffsetsAndStride() {

			uint32_t offset = 0;
			m_Stride = 0;

			for(auto& element : m_Elements) {
				element.Offset = offset;
				offset += static_cast<uint32_t>(element.Size);
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
		size_t m_Stride = 0;

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
		static Ref<VertexBuffer> Create(float* vertices, size_t size);

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

		virtual operator uint32_t() const = 0;
	};



	//////////////////////////////////////////////////////////////////////////////////////////
	/// IndexBuffer //////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

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
		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);

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

		virtual operator uint32_t() const = 0;

	};



	//////////////////////////////////////////////////////////////////////////////////////////
	/// UniformBuffer ////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	// 临时使用（之后会交给Shaderc这个库来完成）
	static uint32_t ShaderDataTypeAlignas(ShaderDataType type) {

		switch (type) {

			case(ShaderDataType::Float):	return (BIT(2) - 1);
			case(ShaderDataType::Float2):	return (BIT(3) - 1);
			case(ShaderDataType::Float3):	return (BIT(4) - 1);
			case(ShaderDataType::Float4):	return (BIT(4) - 1);
			case(ShaderDataType::Int):		return (BIT(2) - 1);
			case(ShaderDataType::Int2):		return (BIT(3) - 1);
			case(ShaderDataType::Int3):		return (BIT(4) - 1);
			case(ShaderDataType::Int4):		return (BIT(4) - 1);
			case(ShaderDataType::Bool):		return (BIT(2) - 1);
			case(ShaderDataType::Mat3):		return (BIT(4) - 1);
			case(ShaderDataType::Mat4):		return (BIT(4) - 1);

		}

		AYIN_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;


	}


	/// <summary>
	/// Block中一个元素的类型，名称，偏移，大小
	/// </summary>
	struct AYIN_API UniformElement {

		UniformElement(ShaderDataType type, const std::string& name)
			:Type{ type }, Name{ name }, Size{ ShaderDataTypeSize(type) }
		{
		};

		ShaderDataType Type;	// 变量类型
		std::string Name;		// 变量名称
		uint32_t Offset = 0;	// 起始偏移
		size_t Size;			// 变量大小

	};


	//! 有时候一个Uniform不一定是一个变量，可能是一个结构体（一个binding槽），由一系列变量组成
	/// <summary>
	/// 仅仅是记录相关总大小数据
	/// </summary>
	class AYIN_API UniformLayout {

	public:

		/// <summary>
		/// 该构造什么都不干
		/// </summary>
		UniformLayout() = default;

		UniformLayout(const std::string& name, const std::initializer_list<UniformElement>& uniformElements)
			:Name{ name }, m_UniformElements{ uniformElements }
		{
			CalculateOffsetAndTotalSize();
		};

		~UniformLayout() = default;

		//计算总大小
		inline void CalculateOffsetAndTotalSize() {

			uint32_t offset = 0;
			m_TotalSize = 0;

			for (auto& element : m_UniformElements) {

				uint32_t aligna = ShaderDataTypeAlignas(element.Type);

				if ((offset & aligna) > uint32_t(0)) {
					offset = offset & (~aligna);//尾部清零
					offset += (aligna + 1);
				}

				element.Offset = offset;

				offset += element.Size;

			}

			m_TotalSize = offset + m_UniformElements.back().Size;
			if ((m_TotalSize & (BIT(4) - 1)) > uint32_t(0)) {
				m_TotalSize = m_TotalSize & ~(BIT(4) - 1);//尾部清零
				m_TotalSize += BIT(4);
			}
			

		};


		inline size_t GetSize() const { return m_TotalSize; };


		inline std::vector<UniformElement>::iterator begin() { return m_UniformElements.begin(); }
		inline std::vector<UniformElement>::iterator end() { return m_UniformElements.end(); }
		inline std::vector<UniformElement>::const_iterator begin() const { return m_UniformElements.begin(); }
		inline std::vector<UniformElement>::const_iterator end() const { return m_UniformElements.end(); }

	public:

		std::string Name;

	private:


		std::vector<UniformElement> m_UniformElements;

		size_t m_TotalSize = 0;

	};


	/// <summary>
	/// 统一变量缓冲（仅支持单Block）
	/// 开辟GPU空间，设置数据，绑定到对应binding槽中
	/// </summary>
	//! 不难发现其实这些buffer基本只是对指令的封装，不会干涉数据
	class AYIN_API UniformBuffer {

	public:

		/// <summary>
		/// 工厂方法
		/// </summary>
		/// <param name="data"></param>
		/// <param name="layout"></param>
		/// <returns></returns>
		static Ref<UniformBuffer> Create(void* data, size_t size);

		virtual ~UniformBuffer() = default;

		///// <summary>
		///// 设置指定数据
		///// </summary>
		///// <param name="paramName"></param>
		///// <param name="data"></param>
		//virtual void Set(const std::string& paramName, void* data) = 0;

		/// <summary>
		/// 全部数据设置
		/// </summary>
		/// <param name="data"></param>
		virtual void Set(void* data) = 0;

		/// <summary>
		/// 布局配置（解释现有内存布局，现有意味着不会因为你告知布局改动而改变内存）
		/// </summary>
		/// <param name="layout"></param>
		virtual void SetLayout(const UniformLayout& layout) = 0;

		/// <summary>
		/// 获取布局
		/// </summary>
		/// <returns></returns>
		virtual const UniformLayout& GetLayout() const = 0;
		//! 一般来说我们不会返回const&，但当方法为const方法时，函数体中的所有成变会被加上const修饰
		//! 这个方法dagailv也是返回成变，在这个函数体中就是返回const，那么你也要一个const返回类型来接
		//! 至于&，是因为当下就两个选择T，const T，const T&，很显然最后一个可以避免拷贝

		virtual void SetBindingIndexs(const std::initializer_list<int>& bindingIndexs) = 0;
		static const std::vector<int>& GetIndexs();

		virtual void Bind() const = 0;

		virtual void UnBind() const = 0;

		virtual operator uint32_t() const = 0;

	};

}