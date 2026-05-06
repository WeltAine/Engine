#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/Entity.h"

#include "Ayin/Core/Timestep.h"

namespace Ayin{


	class ScriptableEntity {

		friend class Scene;

	public:

		ScriptableEntity() = default;
		virtual ~ScriptableEntity() = default;

		virtual void OnCreate() {};
		virtual void OnUpdate(Timestep deltaTime) {};
		virtual void OnDestroy() {};

		template<typename... ComponentTypes>
		decltype(auto) GetComponents();

	private:

		Entity m_Entity;	//脚本所作用的实体

	};


	template<typename... ComponentTypes>
	decltype(auto) ScriptableEntity::GetComponents() {
	
		return m_Entity.GetComponents<ComponentTypes...>();

	};

}