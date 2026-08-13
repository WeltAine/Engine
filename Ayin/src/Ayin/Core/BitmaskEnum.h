#pragma once
#include "Ayin/Core/Core.h"

namespace Ayin {

	//! 通过 operator 隐式调用，模板 + 类型萃取 + 概念 来支持 enum 的位操作
	//! 当对某种枚举类型尝试进行模板操作时会生成对应模板

	template<typename E>
	inline constexpr bool enable_bitmask_operators = false;		// 尝试支持枚举运算

	template<typename E>
	concept BitmaskEnum = std::is_enum_v<E> && enable_bitmask_operators<E>;

	template<BitmaskEnum E>
	constexpr E operator| (E leftEnum, E rightEnum) {
		using UnderlyingType = std::underlying_type_t<E>;	// 得到某个枚举实际使用的整数存储类型
		return static_cast<E>(static_cast<UnderlyingType>(leftEnum) | static_cast<UnderlyingType>(rightEnum));
	};

	template<BitmaskEnum E>
	constexpr E operator& (E leftEnum, E rightEnum) {
		using UnderlyingType = std::underlying_type_t<E>;	// 得到某个枚举实际使用的整数存储类型
		return static_cast<E>(static_cast<UnderlyingType>(leftEnum) & static_cast<UnderlyingType>(rightEnum));
	};

	template<BitmaskEnum E>
	constexpr E operator^ (E leftEnum, E rightEnum) {
		using UnderlyingType = std::underlying_type_t<E>;	// 得到某个枚举实际使用的整数存储类型
		return static_cast<E>(static_cast<UnderlyingType>(leftEnum) ^ static_cast<UnderlyingType>(rightEnum));
	};

	template<BitmaskEnum E>
	constexpr E operator~ (E mode) {
		using UnderlyingType = std::underlying_type_t<E>;	// 得到某个枚举实际使用的整数存储类型
		return static_cast<E>(~static_cast<UnderlyingType>(mode));
	};

	template<BitmaskEnum E>
	constexpr E& operator|= (E& leftEnum, E rightEnum) {
		using UnderlyingType = std::underlying_type_t<E>;	// 得到某个枚举实际使用的整数存储类型
		leftEnum = leftEnum | rightEnum;
		return leftEnum;
	};

	template<BitmaskEnum E>
	constexpr E& operator&= (E& leftEnum, E rightEnum) {
		using UnderlyingType = std::underlying_type_t<E>;	// 得到某个枚举实际使用的整数存储类型
		leftEnum = leftEnum & rightEnum;
		return leftEnum;
		//! static_cast 当转换为非引用类型时会产生拷贝操作，也就是返回一个新值
	};



};
