#pragma once

#include "Ayin/Core/Core.h"
#include "Ayin/Core/Timestep.h"
#include "Ayin/Scene/EditorCamera.h"

#include <entt/entt.hpp>
#include <glm/glm.hpp>
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
		Entity CreateAttachmentEntity(Entity owner);
		void DestroyEntity(const Entity& entity);
		Entity FindEntityByUUID(uint64_t uuid);
		std::vector<Entity> GetAttachmentEntities(Entity owner);

		void SetParent(Entity child, Entity parent, bool keepWorldTransform = true);
		void Unparent(Entity child, bool keepWorldTransform = true);
		std::vector<Entity> GetChildren(Entity entity);
		Entity GetParent(Entity entity);

		glm::mat4 CalculateWorldTransform(Entity entity);

		void OnUpdateRuntime(Timestep deltaTime);
		void OnUpdateEditor(Timestep deltaTime, EditorCamera& editorCamera);

		// 根据展示窗口调整相机比例（是所有相机）
		void OnViewportResize(int width, int height);

		const std::string& GetName() const { return m_SceneName; }
		void SetName(const std::string& name) { m_SceneName = name; }

		template<typename... ComponentTypes, typename... ExcludeComponentTypes>
		std::vector<Entity> GetEntitiesByComponents(entt::exclude_t<ExcludeComponentTypes...> = entt::exclude_t<ExcludeComponentTypes...>{});


	private:

		Entity CreateEntityWithUUID(uint64_t uuid = 0);
		uint64_t GenerateUniqueUUID(uint64_t preferredUUID = 0);
		bool IsUUIDAvailable(uint64_t uuid, entt::entity ignoredEntity = entt::null);
		bool IsValidEntity(Entity entity) const;

		entt::registry m_Registry;
		std::string m_SceneName = "Untitled";

	};

	template<typename... ComponentTypes, typename... ExcludeComponentTypes>
	std::vector<Entity> Scene::GetEntitiesByComponents(entt::exclude_t<ExcludeComponentTypes...>) {
	
		auto&& view = m_Registry.view<ComponentTypes...>(entt::exclude_t<ExcludeComponentTypes...>{});

		std::vector<Entity> entities;

		view.each([&entities, this](entt::entity entity, auto&& ...) {
				entities.emplace_back(entity, this);
			});

		return entities;

	};


}
