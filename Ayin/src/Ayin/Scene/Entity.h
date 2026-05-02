#pragma once

#include "Ayin/Core/Core.h"
#include "Ayin/Scene/Scene.h"

#include <entt/entt.hpp>

namespace Ayin {

	class Entity {

	public:

		Entity() = default;
		Entity(Scene* scene);
		Entity(entt::entity entityHandle, Scene* scene) :m_EntityHandle{ entityHandle }, m_Scene{ scene } {};
		Entity(const Entity& entity) = default;
		~Entity() = default;

		template<typename ComponentType, typename... Args>
		ComponentType& AddComponent(Args&&... args);

		template<typename... ComponentTypes>
		void RemoveComponents();

		template<typename... ComponentTypeArgs>
		bool HasComponent();

		template<typename... ComponentTypes>
		decltype(auto) GetComponents();

		operator const entt::entity&() const { return m_EntityHandle; };
		operator bool() const { return m_EntityHandle != entt::null; }

	private:
		entt::entity m_EntityHandle{ 0 };
		
		Scene* m_Scene = nullptr;
		
	};






	template<typename ComponentType, typename... Args>
	ComponentType& Entity::AddComponent(Args&&... args) {

		AYIN_CORE_ASSERT(!HasComponent<ComponentType>(), "Entity already has component!");

		if (!HasComponent<ComponentType>())
			return m_Scene->m_Registry.emplace<ComponentType>(m_EntityHandle, std::forward<Args>(args)...);

	};

	template<typename... ComponentTypes>
	void Entity::RemoveComponents() {

		AYIN_CORE_ASSERT(HasComponent<ComponentTypes...>(), "Entity doesn't have component!");

		m_Scene->m_Registry.remove<ComponentTypes...>(m_EntityHandle);

	};

	template<typename... ComponentTypes>
	bool Entity::HasComponent() {

		return m_Scene->m_Registry.all_of<ComponentTypes...>(m_EntityHandle);

	};

	template<typename... ComponentTypes>
	decltype(auto) Entity::GetComponents() {

		//这是 C++ 标准委员会定义的硬性规则。对于 decltype(e)，推导结果如下：
		//	如果 e 是一个普通的变量名，推导结果就是该变量定义的类型。
		//	如果 e 是一个表达式，且 e 产生一个左值（lvalue），推导结果就是 T & 。
		//	如果 e 产生一个右值（prvalue，如临时值），推导结果就是 T。

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