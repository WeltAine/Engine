#pragma once

#include "Ayin/Core/Core.h"
#include "Ayin/Scene/Scene.h"

#include <entt/entt.hpp>
#include <concepts>
#include <string>


//ToDo 使用反射库，而不是这样别扭的形式添加数据（一些以id为参数的函数会存在是因为我们没法将id_type转回类型）


namespace Ayin {

	class ScriptableEntity;
	struct NativeScriptComponent;

	//! 概念：检查组件是否有依赖组件
	//! 组件中的依赖通过using Requires = entt::type_list<...>来表达
	template<typename ComponentType>
	concept HasRequirements = requires{ typename ComponentType::Requires; };

	class AYIN_API Entity {

		friend Scene;

	public:

		Entity() = default;
		Entity(Scene* scene);
		Entity(entt::entity entityHandle, Scene* scene) :m_EntityHandle{ entityHandle }, m_Scene{ scene } {};
		Entity(const Entity& entity) = default;
		~Entity() = default;


		template<typename ComponentType, typename... Args>
		ComponentType& AddComponent(Args&&... args);

		template<typename ScriptType>
			requires std::derived_from<ScriptType, ScriptableEntity> && std::default_initializable<ScriptType>
		Entity AddScript();
		Entity AddScript(const std::string& scriptName);

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


		operator const entt::entity&() const { return m_EntityHandle; };
		inline Scene* GetScene() const { return m_Scene; }

		operator uint32_t() const { return (uint32_t)m_EntityHandle; };
		operator bool() const { return m_EntityHandle != entt::null; }

		inline bool operator == (const Entity& other) const { return m_EntityHandle == other.m_EntityHandle; };
		inline bool operator != (const Entity& other) const { return !((*this) == other); };

	private:
		entt::entity m_EntityHandle{ entt::null };
		
		Scene* m_Scene = nullptr;
		
	};



	template<typename ComponentType, typename... Args>
	ComponentType& Entity::AddComponent(Args&&... args) {


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

		AYIN_CORE_ASSERT(HasComponents<ComponentTypes...>(), "Entity doesn't have component!");

		m_Scene->m_Registry.remove<ComponentTypes...>(m_EntityHandle);

	};

	template<typename... ComponentTypes>
	bool Entity::HasComponents() const{

		return m_Scene->m_Registry.all_of<ComponentTypes...>(m_EntityHandle);

	};

	template<typename... ComponentTypes>
	decltype(auto) Entity::GetComponents() {

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
