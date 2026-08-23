#pragma once

#include "Ayin/Core/Core.h"

#include <entt/entt.hpp>


namespace Ayin {
	

	class Entity;
	class Scene;
	
	class AYIN_API ICanChangeScene {


	public:

		// --------------------- 实体操作接口 ---------------------------
		Entity CreateEntity(const std::string& name = "Entity");
		void DestroyEntity(Entity& entity);

		Entity FindEntityByUUID(uint64_t UUID) const;


		// --------------------- 组件操作接口 ---------------------------
		void DestroyComponent(Entity& entity, ::entt::id_type componentId);
		template<typename ComponentType>
		void DestroyComponent(Entity& entity);

		// ----------------------------父子关系接口------------------------------------
		void SetParent(Entity& child, Entity& parent, bool keepWorldTransform);
		void UnParent(Entity& child, bool keepWorldTransform);

		Entity GetParent(const Entity& child) const;
		std::vector<Entity> GetChilds(const Entity& parent) const;

		// 是否是后代
		bool IsDescendant(const Entity& entity, const Entity& parent);


		// ----------------------------- 遍历操作 ---------------------------------
		template<typename... ComponentTypes, typename Func, typename... ExcludeComponentTypes>
			requires std::invocable<Func&, entt::entity, ComponentTypes&...>
		void Each(Func&& func, entt::exclude_t<ExcludeComponentTypes...> exclude = entt::exclude_t<ExcludeComponentTypes...>{});

		template<typename... ComponentTypes, typename Func, typename... ExcludeComponentTypes>
			requires (std::invocable<Func&, ComponentTypes&...> &&
		!std::invocable<Func&, entt::entity, ComponentTypes&...>)
			void Each(Func&& func, entt::exclude_t<ExcludeComponentTypes...> exclude = entt::exclude_t<ExcludeComponentTypes...>{});

		// 遍历访问元素
		template<typename... ComponentTypes, typename Func, typename... ExcludeComponentTypes>
			requires std::invocable<Func&, entt::entity, const ComponentTypes&...>
		void ConstEach(Func&& func, entt::exclude_t<ExcludeComponentTypes...> exclude = entt::exclude_t<ExcludeComponentTypes...>{}) const;

		template<typename... ComponentTypes, typename Func, typename... ExcludeComponentTypes>
			requires (std::invocable<Func&, const ComponentTypes&...> &&
		!std::invocable<Func&, entt::entity, const ComponentTypes&...>)
			void ConstEach(Func&& func, entt::exclude_t<ExcludeComponentTypes...> exclude = entt::exclude_t<ExcludeComponentTypes...>{}) const;



	protected:

		virtual View<Scene> GetScene() = 0;


	};



	template<typename ComponentType>
	void ICanChangeScene::DestroyComponent(Entity& entity) {
	
		GetScene()->DestroyComponent<ComponentType>(entity);

	};


	template<typename... ComponentTypes, typename Func, typename... ExcludeComponentTypes>
		requires std::invocable<Func&, entt::entity, ComponentTypes&...>
	void ICanChangeScene::Each(Func&& func, entt::exclude_t<ExcludeComponentTypes...> exclude) {
	
		GetScene()->Each<ComponentTypes...>(std::forward<Func>(func), exclude);
	
	};

	template<typename... ComponentTypes, typename Func, typename... ExcludeComponentTypes>
		requires (std::invocable<Func&, ComponentTypes&...> &&
	!std::invocable<Func&, entt::entity, ComponentTypes&...>)
		void ICanChangeScene::Each(Func&& func, entt::exclude_t<ExcludeComponentTypes...> exclude) {
	
		GetScene()->Each<ComponentTypes...>(std::forward<Func>(func), exclude);
	
	};

	template<typename... ComponentTypes, typename Func, typename... ExcludeComponentTypes>
		requires std::invocable<Func&, entt::entity, const ComponentTypes&...>
	void ICanChangeScene::ConstEach(Func&& func, entt::exclude_t<ExcludeComponentTypes...> exclude) const {
	
		GetScene()->Each<ComponentTypes...>(std::forward<Func>(func), exclude);

	};

	template<typename... ComponentTypes, typename Func, typename... ExcludeComponentTypes>
		requires (std::invocable<Func&, const ComponentTypes&...> &&
	!std::invocable<Func&, entt::entity, const ComponentTypes&...>)
		void ICanChangeScene::ConstEach(Func&& func, entt::exclude_t<ExcludeComponentTypes...> exclude) const {
	
		GetScene()->Each<ComponentTypes...>(std::forward<Func>(func), exclude);

	};


}
