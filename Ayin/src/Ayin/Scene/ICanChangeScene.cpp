#include <AyinPch.h>

#include "Ayin/Scene/ICanChangeScene.h"

#include "Ayin/Scene/Scene.h"
#include "Ayin/Scene/Entity.h"

namespace Ayin {

	// --------------------- 实体操作接口 ---------------------------
	Entity ICanChangeScene::CreateEntity(const std::string& name) {
	
		return GetScene()->CreateEntity(name);

	};

	void ICanChangeScene::DestroyEntity(Entity& entity) {
	
		GetScene()->DestroyEntity(entity);

	};

	Entity ICanChangeScene::FindEntityByUUID(uint64_t UUID) const {
	
		return GetScene()->FindEntityByUUID(UUID);

	};


	// --------------------- 组件操作接口 ---------------------------
	void ICanChangeScene::DestroyComponent(Entity& entity, ::entt::id_type componentId) {
	
		GetScene()->DestroyComponent(entity, componentId);

	};

	// ----------------------------父子关系接口------------------------------------
	void ICanChangeScene::SetParent(Entity& child, Entity& parent, bool keepWorldTransform) {
		
		GetScene()->SetParent(child, parent, keepWorldTransform);
	
	};

	void ICanChangeScene::UnParent(Entity& child, bool keepWorldTransform) {
	
		GetScene()->UnParent(child, keepWorldTransform);
	
	};

	Entity ICanChangeScene::GetParent(const Entity& child) const {
	
		return GetScene()->GetParent(child);

	};

	std::vector<Entity> ICanChangeScene::GetChilds(const Entity& parent) const {
	
		return GetScene()->GetChilds(parent);
	
	};

	// 是否是后代
	bool ICanChangeScene::IsDescendant(const Entity& entity, const Entity& parent) {
	
		GetScene()->IsDescendant(entity, parent);
	
	};


}
