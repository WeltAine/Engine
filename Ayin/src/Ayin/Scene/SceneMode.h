#pragma once

#include "Ayin/Core/BitmaskEnum.h"
#include "Ayin/Core/Core.h"

#include <concepts>
#include <cstdint>


namespace Ayin {

	// 场景模式既描述 World 的运行状态，也作为 System 的模式筛选 mask。
	enum class SceneMode : uint8_t {

		None = 0,
		Editor = BIT(0),
		Simulation = BIT(1),
		Runtime = BIT(2),

		AllSceneMode = BIT(3) - 1

	};

	//! 模板变量特化
	template<>
	inline constexpr bool enable_bitmask_operators<SceneMode> = true;


	//! C++20 的 abbreviated function template。带初始值的二元折叠表达式允许空参数调用。
	constexpr SceneMode ToMask(std::same_as<SceneMode> auto... modes) {

		return (SceneMode{ 0 } | ... | static_cast<SceneMode>(modes));

	};

	inline bool Contains(SceneMode mask, SceneMode mode) { return static_cast<bool>(mask & mode); }


};
