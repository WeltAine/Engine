#include <AyinPch.h>

#include "Ayin/Scene/Entity.h"
#include "Ayin/Scene/Components.h"

namespace Ayin {


	Entity::Entity(Scene* scene) 
		:m_Scene{scene}
	{
		if (m_Scene != nullptr) {
			m_EntityHandle = m_Scene->m_Registry.create();
			m_SceneLifetime = m_Scene->m_LifetimeToken;
		}

	};

	Entity::Entity(entt::entity entityHandle, Scene* scene)
		:m_EntityHandle{ entityHandle }, m_Scene{ scene } 
	{
		if (m_Scene == nullptr) {
			m_EntityHandle = ::entt::null;
			return;
		}

		m_SceneLifetime = scene->m_LifetimeToken;
	};


	// ----------------世界或本地姿态矩阵------------------
	glm::mat4 Entity::GetWorldTransform() const {

		if(m_EntityHandle == ::entt::null || m_SceneLifetime.expired())
			return glm::mat4{ 1.0f };


		if (!HasComponents<TransformComponent>())
			return glm::mat4{1.0f};

		TransformComponent transformComponent = GetComponents<TransformComponent>();
		
		glm::mat4 worldMatrix{1.0f};// 局部矩阵

		Entity parent = *this;

		do {
		
			TransformComponent& transformComponent = parent.GetComponents<TransformComponent>();
			worldMatrix = glm::mat4(transformComponent) * worldMatrix;

			parent = m_Scene->GetParent(parent);

		} while (parent);

		return worldMatrix;
	
	};

	glm::mat4 Entity::GetLocalTransform() const {
	
		AYIN_CORE_ASSERT(!m_SceneLifetime.expired(), "Scene and entity resources missing");


		if (!HasComponents<TransformComponent>())
			return glm::mat4{ 1.0f };

		return glm::mat4(GetComponents<TransformComponent>());

	};
	// ----------------------------------------------------


	Entity Entity::GetParent() const {
	
		AYIN_CORE_ASSERT(!m_SceneLifetime.expired(), "Scene and entity resources missing");

		return m_Scene->GetParent(*this);

	};

	std::vector<Entity> Entity::GetChilds() const {
		
		AYIN_CORE_ASSERT(!m_SceneLifetime.expired(), "Scene and entity resources missing");

		return m_Scene->GetChilds(*this);
	
	};


	bool Entity::IsDescendantOf(const Entity& parent) const {
	
		AYIN_CORE_ASSERT(!m_SceneLifetime.expired(), "Scene and entity resources missing");

		return m_Scene->IsDescendant(*this, parent);
	
	};

	

	bool Entity::HasComponent(entt::id_type id) const {

		AYIN_CORE_ASSERT(!m_SceneLifetime.expired(), "Scene and entity resources missing");


		auto* componentPool = m_Scene->m_Registry.storage(id);

		if (!componentPool)
			return false;

		return componentPool->contains(m_EntityHandle);

	};

	void Entity::RemoveComponent(entt::id_type id) {

		AYIN_CORE_ASSERT(!m_SceneLifetime.expired(), "Scene and entity resources missing");


		auto* componentPool = m_Scene->m_Registry.storage(id);
		if (componentPool) {
			componentPool->remove(m_EntityHandle);
		}

	}
	

}
