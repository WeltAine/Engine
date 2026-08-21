#pragma once

#include "Ayin/Core/BitmaskEnum.h"
#include "Ayin/Core/Core.h"

#include "Ayin/Scene/SceneMode.h"
#include "Ayin/System/ISystem.h"

#include <concepts>
#include <cstdint>
#include <string>

#include <entt/entt.hpp>


namespace Ayin {

	enum class AYIN_API SystemPhase : uint8_t {

		None = 0,

		PreUpdate = BIT(0),
		Update = BIT(1),
		PostUpdate = BIT(2),
		Presentation = BIT(3)

	};

	template<>
	inline constexpr bool enable_bitmask_operators<SystemPhase> = true;

	using SystemType = entt::id_type;
	using SystemID = SystemType;

	template<typename System>
		requires std::derived_from<System, ISystem>
	[[nodiscard]] SystemType GetSystemType() noexcept {
		return entt::type_hash<System>::value();
	};

	template<typename System>
		requires std::derived_from<System, ISystem>
	[[nodiscard]] SystemID GetSystemID() noexcept {
		return entt::type_hash<System>::value();
	};

	struct SystemSpecification {

		SystemPhase PhaseMask = SystemPhase::None;
		SceneMode ModeMask = SceneMode::None;
		int Order = -1;									// -1 表示不指定顺序（将由程序以自动递加的顺序设定）

	};

	// TypeKey 是定义在项目文件中的稳定身份；运行时实例不需要保存 Registry 描述符。
	using SystemTypeKey = std::string;

	// System 的可持久化配置先以 JSON 文本保存，领域模型不因此依赖 Glaze。
	struct SystemConfiguration {

		std::string Json = "{}";

	};

	// Pipeline 中的一个结构定义，不包含 System 实例，也不包含运行时身份。
	struct SystemDefinition {

		SystemTypeKey Type;
		SystemSpecification Specification;
		SystemConfiguration Configuration;

	};

	// Schedule 内部使用的运行时信息，TypeKey 用于观察、诊断和导出。
	struct SystemInformation {

		SystemID RuntimeId;
		SystemTypeKey TypeKey;

	};


};
