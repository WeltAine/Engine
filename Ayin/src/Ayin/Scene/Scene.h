#pragma once

#include "Ayin/Core/Core.h"
#include "Ayin/Core/Timestep.h"
#include "Ayin/Scene/EditorCamera.h"
#include "Ayin/Scene/SceneMode.h"

#include <entt/entt.hpp>
#include <string>
#include <vector>


namespace Ayin {

	class Entity;

	struct DestroyComponent;

	// ----------------------------------------------------------------------------------------------------------------

	class AYIN_API Scene {

		friend class Entity;
		friend class SceneSerializer;

		friend class DestroySystem;

	public:

		Scene() = default;
		~Scene();

		// 创建一个只有 UUID 的实体
		Entity CreateUUIDEntity();

		// 创建一个可交互的基础实体
		Entity CreateEntity(const std::string& name = "Entity");
		void DestroyEntity(Entity& entity);
		void DestroyComponent(Entity& entity, ::entt::id_type componentId);

		template<typename ComponentType>
		void DestroyComponent(Entity& entity);


		// ----------------------------父子关系接口------------------------------------
		void SetParent(Entity& child, Entity& parent, bool keepWorldTransform);
		void UnParent(Entity& child, bool keepWorldTransform);

		Entity GetParent(const Entity& child) const;
		std::vector<Entity> GetChilds(const Entity& parent) const;

		// 是否是后代
		static bool IsDescendant(const Entity& entity, const Entity& parent);

		// ----------------------------------------------------------------------------
		void OnUpdateRuntime(Timestep deltaTime);
		void OnUpdateSimulation(Timestep deltaTime, EditorCamera& editorCamera);
		void OnUpdateEditor(Timestep deltaTime, EditorCamera& editorCamera);

		// 根据展示窗口调整相机比例（是所有相机）
		void OnViewportResize(int width, int height);

		// ----------------------------------------------------------------------------
		const std::string& GetName() const { return m_SceneName; }
		void SetName(const std::string& name) { m_SceneName = name; }

		template<typename... ComponentTypes, typename... ExcludeComponentTypes>
		std::vector<Entity> GetEntitiesByComponents(entt::exclude_t<ExcludeComponentTypes...> = entt::exclude_t<ExcludeComponentTypes...>{});


		// 遍历操作元素
		//! 如果你的一个类型参数是空类型，那么请不要在回调中添加该空类型参数，因为 EnTT 的空类型优化不会返回空类型的引用，空类型筛选还是没问题的，请放心（目前我不太想管这个问题，我现在甚至都要求使用者回调的第一个参数是 entt::entity 而不是 Entity，实际上在实现中我尝试了使用 lambda 再包装一次，不过空类型的问题还在就放弃了，所以这些问题就先抛给使用者了）
		template<typename... ComponentTypes, typename Func, typename... ExcludeComponentTypes>
			requires std::invocable<Func&, entt::entity, ComponentTypes&...>
		void Each(Func&& func, entt::exclude_t<ExcludeComponentTypes...> = entt::exclude_t<ExcludeComponentTypes...>{});

		// 遍历访问元素
		template<typename... ComponentTypes, typename Func, typename... ExcludeComponentTypes>
			requires std::invocable<Func&, entt::entity, const ComponentTypes&...>
		void ConstEach(Func&& func, entt::exclude_t<ExcludeComponentTypes...> = entt::exclude_t<ExcludeComponentTypes...>{}) const;


		Entity FindEntityByUUID(uint64_t UUID) const;


	private:
		//Todo: 准备移除
		void SubmitEntityDestroy(const Entity& entity) {};
		//Todo: 准备移除
		void InternalDestroyEntity(Entity& entity) {};
		//Todo: 准备移除
		void FlushDestroyedEntities() {};


	private:

		entt::registry m_Registry;
		std::string m_SceneName = "Untitled";

		std::shared_ptr<void> m_LifetimeToken = std::make_shared<int>(0);

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



	template<typename... ComponentTypes, typename Func, typename... ExcludeComponentTypes>
		requires std::invocable<Func&, entt::entity, ComponentTypes&...>
		//! 一些成员函数对实例的值类别是有要求的，方法就是像 const 修饰一样，在同样的位置写 & 或者 && 就可以开启（我记得我前段时间还用过来着）
		//! 而 std::invocable<Func& ...... 就是在强调以左值进行 invoke 检测（T& 必然是左值），之所以这么做是因为 view.each() 内部就是以左值进行调用
	void Scene::Each(Func&& func, entt::exclude_t<ExcludeComponentTypes...>) {
	
		//auto lambda = [&func, this](entt::entity entity, ComponentTypes&... components) {
		//	func(Entity{entity, this}, components...);
		//	};


		auto&& view = m_Registry.view<ComponentTypes...>(entt::exclude_t<ExcludeComponentTypes...>{});

		view.each(std::forward<Func>(func));
		//! std::forward 不是“模板函数里调用另一个函数时通常都要用”.
		//! 它只适用于一种明确场景：当前函数是一个转发层，并且下层函数应该保留调用者传入参数的左值 / 右值属性。

	};

	template<typename... ComponentTypes, typename Func, typename... ExcludeComponentTypes>
		requires std::invocable<Func&, entt::entity, const ComponentTypes&...>
		//! 一些成员函数对实例的值类别是有要求的，方法就是像 const 修饰一样，在同样的位置写 & 或者 && 就可以开启（我记得我前段时间还用过来着）
		//! 而 std::invocable<Func& ...... 就是在强调以左值进行 invoke 检测（T& 必然是左值），之所以这么做是因为 view.each() 内部就是以左值进行调用
	void Scene::ConstEach(Func&& func, entt::exclude_t<ExcludeComponentTypes...>) const {
	
		//auto lambda = [&func, this](entt::entity entity, ComponentTypes&... components) {
		//	func(Entity{entity, this}, components...);
		//	};


		auto&& view = m_Registry.view<ComponentTypes...>(entt::exclude_t<ExcludeComponentTypes...>{});

		view.each(std::forward<Func>(func));
		//! std::forward 不是“模板函数里调用另一个函数时通常都要用”.
		//! 它只适用于一种明确场景：当前函数是一个转发层，并且下层函数应该保留调用者传入参数的左值 / 右值属性。


	};


	template<typename ComponentType>
	void Scene::DestroyComponent(Entity& entity) {

		DestroyComponent(entity, ComponentType::ComponentStorageID());

	};


}
