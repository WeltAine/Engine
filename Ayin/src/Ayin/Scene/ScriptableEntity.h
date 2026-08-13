#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/Entity.h"

#include "Ayin/Core/Timestep.h"

#include "Ayin/System/Systems.h"

#include <optional>
#include <string>
#include <entt/entt.hpp>

namespace Ayin{


	class ScriptableEntity {

		friend class Scene;
		friend class SceneSerializer;
		friend struct NativeScriptComponent;
		friend class ScriptSystem;

	public:

		ScriptableEntity() = default;
		virtual ~ScriptableEntity() = default;

		inline void SetEntity(const Entity& entity) { m_Entity = entity; };

		virtual inline std::optional<std::string> GetScriptName() const { return std::nullopt; };
		virtual inline std::optional<entt::id_type> GetScriptID() const { return std::nullopt; };


		//脚本绘制
		virtual void OnGui() {};

		template<typename... ComponentTypes>
		decltype(auto) GetComponents();

	protected:
		virtual void OnCreate() {};
		virtual void OnUpdate(Timestep deltaTime) {};
		virtual void OnDestroy() {};

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
