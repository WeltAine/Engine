#include <AyinPch.h>

#include "Ayin/Scene/Entity.h"
#include "Ayin/Scene/Components.h"
#include "Ayin/Scene/ScriptRegistry.h"

namespace Ayin {


	Entity::Entity(Scene* scene) 
		:m_Scene{scene}
	{
	
		if (m_Scene != nullptr)
			m_EntityHandle = m_Scene->m_Registry.create();

	};

	

	bool Entity::HasComponent(entt::id_type id) const {

		auto* componentPool = m_Scene->m_Registry.storage(id);

		if (!componentPool)
			return false;

		return componentPool->contains(m_EntityHandle);

	};

	void Entity::RemoveComponent(entt::id_type id) {
		auto* componentPool = m_Scene->m_Registry.storage(id);
		if (componentPool) {
			componentPool->remove(m_EntityHandle);
		}
	}

	Entity Entity::AddScript(const std::string& scriptName) {

		if (!m_Scene || !HasComponents<IDComponent>()) {
			return {};
		}

		Entity scriptEntity = m_Scene->CreateAttachmentEntity(*this);
		if (!scriptEntity) {
			return {};
		}

		NativeScriptComponent& nsc = scriptEntity.AddComponent<NativeScriptComponent>();
		if (!ScriptRegistry::BindScriptByScriptName(nsc, scriptName)) {
			m_Scene->DestroyEntity(scriptEntity);
			return {};
		}

		return scriptEntity;

	}
	

}
