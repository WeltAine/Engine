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

	template<BitmaskEnum E>
	constexpr E& Synthesis(const std::vector<E>& elements) {
		
		E result = 0;

		for (E& element : elements) {
			result |= element;
		}

		return result;
	
	}

	template<BitmaskEnum E>
	constexpr std::vector<E> Disassemble(E mask) {
	
		using UnderlyingType = std::underlying_type_t<E>;
		using UnsignedType = std::make_unsigned_t<UnderlyingType>;// 把某个整数类型转换成对应的无符号整数类型。 int -> unsigned int
		//? 为何要转换成无符号类型
		//! 有符号整数通常会把最高位当作符号位。但位掩码中的每一位都应该只是一个独立的 bit，不应该被解释成正数或负数。
		//! 使用无符号类型可以明确表示“这是位模式”，而不是普通的有符号数值。

		constexpr std::size_t bitCount = std::numeric_limits<UnsignedType>::digits;	// 它表示某个数值类型有多少个有效二进制位。

		std::vector<E> result;

		for (std::size_t bitIndex = 0; bitIndex < bitCount; ++bitIndex) {

			// 当前尝试的值：0001、0010、0100、1000 ...
			const UnsignedType currentBit =
				static_cast<UnsignedType>(1) << bitIndex;

			// mask 中包含当前 bit
			if ((mask & currentBit) != 0) {
				result.push_back(static_cast<E>(currentBit));
			}
		}

		return result;
	
	}


};
