#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/Entity.h"

#include "Ayin/Core/Timestep.h"

#include <optional>
#include <string>
#include <entt/entt.hpp>

namespace Ayin{


	class ScriptableEntity {

		friend class Scene;
		friend class SceneSerializer;
		friend struct NativeScriptComponent;

	public:

		ScriptableEntity() = default;
		virtual ~ScriptableEntity() = default;

		virtual std::optional<std::string> GetScriptName() const { return std::nullopt; };
		virtual std::optional<entt::id_type> GetScriptID() const { return std::nullopt; };

		virtual void OnCreate() {};
		virtual void OnUpdate(Timestep deltaTime) {};
		virtual void OnDestroy() {};

		//脚本绘制
		virtual void OnGui() {};

		template<typename... ComponentTypes>
		decltype(auto) GetComponents();

	private:

		Entity m_Entity;	//脚本所作用的实体

	};


	template<typename... ComponentTypes>
	decltype(auto) ScriptableEntity::GetComponents() {
	
		return m_Entity.GetComponents<ComponentTypes...>();

	};


#define AYIN_CUSTOM_SCRIPT_TYPE(ScriptType)\
		virtual std::optional<std::string> GetScriptName() const override { return #ScriptType; }\
		virtual std::optional<entt::id_type> GetScriptID() const override { return ::entt::type_hash<ScriptType>::value(); }


}
