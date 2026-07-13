#pragma once

#include "Ayin/Core/Core.h"
#include "Ayin/Core/Timestep.h"
#include "Ayin/Scene/EditorCamera.h"

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

		// 创建一个只有 UUID 的实体
		Entity CreateUUIDEntity();

		// 创建一个可交互的基础实体
		Entity CreateEntity(const std::string& name = "Entity");
		void DestroyEntity(Entity& entity);


		// ----------------------------父子关系接口------------------------------------
		void SetParent(Entity& child, Entity& parent, bool keepWorldTransform);
		void UnParent(Entity& child, bool keepWorldTransform);

		Entity GetParent(const Entity& child) const;
		std::vector<Entity> GetChilds(const Entity& parent) const;

		// 是否是后代
		static bool IsDescendant(const Entity& entity, const Entity& parent);

		// ----------------------------------------------------------------------------
		void OnUpdateRuntime(Timestep deltaTime);
		void OnUpdateEditor(Timestep deltaTime, EditorCamera& editorCamera);

		// 根据展示窗口调整相机比例（是所有相机）
		void OnViewportResize(int width, int height);

		// ----------------------------------------------------------------------------
		const std::string& GetName() const { return m_SceneName; }
		void SetName(const std::string& name) { m_SceneName = name; }

		template<typename... ComponentTypes, typename... ExcludeComponentTypes>
		std::vector<Entity> GetEntitiesByComponents(entt::exclude_t<ExcludeComponentTypes...> = entt::exclude_t<ExcludeComponentTypes...>{});

		Entity FindEntityByUUID(uint64_t UUID) const;


	private:
		void SubmitEntityDestroy(const Entity& entity);
		void InternalDestroyEntity(Entity& entity);
		void FlushDestroyedEntities();


	private:

		entt::registry m_Registry;
		std::string m_SceneName = "Untitled";

		std::shared_ptr<void> m_LifetimeToken = std::make_shared<int>(0);

		std::unordered_set<entt::entity> m_DestroyEntities{};
		//! set 是排序树，而 unorderer_set 是哈希表，它会用到 hash 与 ==

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
