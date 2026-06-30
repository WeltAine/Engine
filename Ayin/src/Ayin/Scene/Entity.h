#pragma once

#include "Ayin/Core/Core.h"
#include "Ayin/Scene/Scene.h"

#include <entt/entt.hpp>

#include <cstddef>
#include <functional>

//ToDo 使用反射库，而不是这样别扭的形式添加数据（一些以id为参数的函数会存在是因为我们没法将id_type转回类型）


namespace Ayin {

	struct EntityHash;


	//! 概念：检查组件是否有依赖组件
	//! 组件中的依赖通过using Requires = entt::type_list<...>来表达
	template<typename ComponentType>
	concept HasRequirements = requires{ typename ComponentType::Requires; };

	class AYIN_API Entity {

		friend class Scene;
		friend struct EntityHash;

	public:

		Entity() = default;
		Entity(Scene* scene);
		Entity(entt::entity entityHandle, Scene* scene);
		Entity(const Entity& entity) = default;
		~Entity() = default;

		// ----------------世界或本地姿态矩阵------------------
		glm::mat4 GetWorldTransform() const;
		glm::mat4 GetLocalTransform() const;
		// ----------------------------------------------------


		Entity GetParent() const;
		std::vector<Entity> GetChilds() const;

		bool IsDescendantOf(const Entity& parent) const;


		template<typename ComponentType, typename... Args>
		ComponentType& AddComponent(Args&&... args);

		/// <summary>
		/// 加载组件所依赖的n个前置组件
		/// </summary>
		/// <typeparam name="...Dependences"></typeparam>
		/// <param name=""></param>
		template<typename... Dependences>
		void AddDependencies(entt::type_list<Dependences...>);

		template<typename... ComponentTypes>
		void RemoveComponents();
		void RemoveComponent(entt::id_type id);

		template<typename... ComponentTypeArgs>
		bool HasComponents() const;
		bool HasComponent(entt::id_type id) const;

		template<typename... ComponentTypes>
		decltype(auto) GetComponents();

		template<typename... ComponentTypes>
		decltype(auto) GetComponents() const;



		inline operator const entt::entity&() const { return m_EntityHandle; };

		inline operator uint32_t() const { return (uint32_t)m_EntityHandle; };
		inline operator bool() const { return m_SceneLifetime.expired() ? false : m_Scene->m_Registry.valid(m_EntityHandle); }

		inline bool operator == (const Entity& other) const { return m_Scene == other.m_Scene && m_EntityHandle == other.m_EntityHandle; };
		inline bool operator != (const Entity& other) const { return !((*this) == other); };
		inline bool operator < (const Entity& other) const { return uint32_t(*this) < uint32_t(other); }

	private:
		entt::entity m_EntityHandle{ entt::null };
		
		Scene* m_Scene = nullptr;
		std::weak_ptr<void> m_SceneLifetime{};
		
	};



	struct EntityHash {

		//x std::unordered_set<Entity, EntityHash> m_DestroyEntities{};
		//! set 是排序树，而 unorderer_set 是哈希表，它会用到 hash 与 ==

		size_t operator()(const Entity& entity) const noexcept {
			size_t sceneHash = std::hash<Scene*>{}(entity.m_Scene);
			size_t handleHash = std::hash<uint32_t>{}((uint32_t)entity);
			return sceneHash ^ (handleHash + 0x9e3779b9 + (sceneHash << 6) + (sceneHash >> 2));
		}

		// - Boost hash_combine
		// - The Art of Computer Programming multiplicative hashing
		// - Knuth 2654435761 golden ratio hash
		// - 0x9e3779b9 golden ratio hash
		// - MurmurHash finalizer，更现代的 bit mixing 思路
		//	如果想深入，Donald Knuth 的《The Art of Computer Programming, Volume 3: Sorting and Searching》里有乘法散列相关背景。Boost 的 hash_combine 文档 / 源码则是这个公式最直接的出处。
	};



	template<typename ComponentType, typename... Args>
	ComponentType& Entity::AddComponent(Args&&... args) {

		AYIN_CORE_ASSERT(!m_SceneLifetime.expired(), "Scene and entity resources missing");

		
		if (!HasComponents<ComponentType>()) {

			//通过模板来自动根据组件的前置组件类型需求，生成模板
			if constexpr (HasRequirements<ComponentType>) {
				AddDependencies(typename ComponentType::Requires{});
			}

			return m_Scene->m_Registry.emplace<ComponentType>(m_EntityHandle, std::forward<Args>(args)...);

		}
		
		return GetComponents<ComponentType>();

	};

	template<typename... Dependencies>
	void Entity::AddDependencies(entt::type_list<Dependencies...>) {
		//借由模板类型自动推导进行类型拆包
		//用折叠语句展开处理每一个组件类型
		(AddComponent<Dependencies>(), ...);

	};



	template<typename... ComponentTypes>
	void Entity::RemoveComponents() {

		AYIN_CORE_ASSERT(!m_SceneLifetime.expired(), "Scene and entity resources missing");

		
		AYIN_CORE_ASSERT(HasComponents<ComponentTypes...>(), "Entity doesn't have component!");
		m_Scene->m_Registry.remove<ComponentTypes...>(m_EntityHandle);

	};

	template<typename... ComponentTypes>
	bool Entity::HasComponents() const{

		AYIN_CORE_ASSERT(!m_SceneLifetime.expired(), "Scene and entity resources missing");

		return m_Scene->m_Registry.all_of<ComponentTypes...>(m_EntityHandle);

	};

	template<typename... ComponentTypes>
	decltype(auto) Entity::GetComponents() {

		AYIN_CORE_ASSERT(!m_SceneLifetime.expired(), "Scene and entity resources missing");


		//这是 C++ 标准委员会定义的硬性规则。对于 decltype(e)，推导结果如下：
		//	如果 e 是一个普通的变量名，推导结果就是该变量定义的类型。
		//	如果 e 是一个表达式，且 e 产生一个左值（lvalue），推导结果就是 T & 。
		//	如果 e 产生一个右值（prvalue，如临时值），推导结果就是 T。

		AYIN_CORE_ASSERT(HasComponents<ComponentTypes...>(),
			"Entity does not have all requested components!");

		if constexpr (sizeof...(ComponentTypes) == 1u) {
			// 最终是一个表达式，且为左值，返回T&
			return *(m_Scene->m_Registry.try_get<ComponentTypes...>(m_EntityHandle));
			// 单个组件：直接解引用指针，返回引用 (T&)
		}
		else {
			// 最终是一个表达式，但结果非左值而是产生右值，所以返回T
			// std::forward_as_tuple会将参数通过完美转发转进行转换并组合成tuple，显然参数是左值（指针），所以产生的tuple以T&为元素
			return std::forward_as_tuple(*(m_Scene->m_Registry.try_get<ComponentTypes>(m_EntityHandle))...);
			// 多个组件：构造一个装满引用的 tuple 并按值返回
		}

		//! if constexpr：编译期分支，在编译时只有有一个分支留下
		//! 所以这里不会遇到 auto 限制（各种情况的返回指类型要求一致）

	};

	template<typename... ComponentTypes>
	decltype(auto) Entity::GetComponents() const {

		AYIN_CORE_ASSERT(!m_SceneLifetime.expired(), "Scene and entity resources missing");


		//这是 C++ 标准委员会定义的硬性规则。对于 decltype(e)，推导结果如下：
		//	如果 e 是一个普通的变量名，推导结果就是该变量定义的类型。
		//	如果 e 是一个表达式，且 e 产生一个左值（lvalue），推导结果就是 T & 。
		//	如果 e 产生一个右值（prvalue，如临时值），推导结果就是 T。

		AYIN_CORE_ASSERT(HasComponents<ComponentTypes...>(),
			"Entity does not have all requested components!");

		if constexpr (sizeof...(ComponentTypes) == 1u) {
			// 最终是一个表达式，且为左值，返回T&
			return *(m_Scene->m_Registry.try_get<ComponentTypes...>(m_EntityHandle));
			// 单个组件：直接解引用指针，返回引用 (T&)
		}
		else {
			// 最终是一个表达式，但结果非左值而是产生右值，所以返回T
			// std::forward_as_tuple会将参数通过完美转发转进行转换并组合成tuple，显然参数是左值（指针），所以产生的tuple以T&为元素
			return std::forward_as_tuple(*(m_Scene->m_Registry.try_get<ComponentTypes>(m_EntityHandle))...);
			// 多个组件：构造一个装满引用的 tuple 并按值返回
		}

	};

}
