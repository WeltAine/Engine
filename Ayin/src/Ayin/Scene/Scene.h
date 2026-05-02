#pragma once

#include "Ayin/Core/Core.h"
#include "Ayin/Core/Timestep.h"

#include <entt/entt.hpp>
#include <string>


namespace Ayin {

	class Entity;

	class Scene {

		friend class Entity;

	public:

		Scene() = default;
		~Scene() = default;

		Entity CreateEntity(const std::string& name = "Entity");

		void OnUpdate(Timestep deltaTime);

	private:

		entt::registry m_Registry;

	};

}