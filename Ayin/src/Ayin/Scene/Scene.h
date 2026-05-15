#pragma once

#include "Ayin/Core/Core.h"
#include "Ayin/Core/Timestep.h"

#include <entt/entt.hpp>
#include <string>
#include <vector>


namespace Ayin {

	class Entity;

	class AYIN_API Scene {

		friend class Entity;
		friend class SceneSerializer;

	public:

		Scene() = default;
		~Scene() = default;

		Entity CreateEntity(const std::string& name = "Entity");
		void DestroyEntity(const Entity& entity);

		void OnUpdate(Timestep deltaTime);

		const std::string& GetName() const { return m_SceneName; }
		void SetName(const std::string& name) { m_SceneName = name; }

		template<typename... ComponentTypes, typename... ExcludeComponentTypes>
		std::vector<Entity> GetEntitiesByComponents(entt::exclude_t<ExcludeComponentTypes...> = entt::exclude_t<ExcludeComponentTypes...>{});


	private:

		entt::registry m_Registry;
		std::string m_SceneName = "Untitled";

	};

	template<typename... ComponentTypes, typename... ExcludeComponentTypes>
	std::vector<Entity> Scene::GetEntitiesByComponents(entt::exclude_t<ExcludeComponentTypes...>) {
	
		auto&& view = m_Registry.view<ComponentTypes...>(entt::exclude_t<ExcludeComponentTypes...>{});

		std::vector<Entity> entities;
		entities.reserve(view.size());

		view.each([&entities, this](entt::entity entity, auto&& ...) {
				entities.emplace_back(entity, this);
			});

		return entities;

	};


}
