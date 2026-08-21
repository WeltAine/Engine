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


	// 系统信息（运行时 ID 和名称）
	struct SystemInformation {

		SystemID RuntimeId;

		std::string Name;

	};

	// 系统配置
	struct SystemSpecification {

		SystemPhase PhaseMask = SystemPhase::None;
		SceneMode ModeMask = SceneMode::None;
		int Order = -1;									// -1 表示不指定顺序（将由程序以自动递加的顺序设定）

	};


};
