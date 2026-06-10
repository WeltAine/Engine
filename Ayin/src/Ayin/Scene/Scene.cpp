#include <AyinPch.h>

#include <glm/glm.hpp>

#include <algorithm>
#include <random>
#include <unordered_set>

#include "Ayin/Scene/Scene.h"
#include "Ayin/Scene/Entity.h"

#include "Ayin/Scene/Components.h"
#include "Ayin/Scene/Systems.h"

#include "Ayin/Math/Math.h"

#include "Ayin/Renderer/Renderer2D.h"

namespace {

	uint64_t GenerateUUID() {

		// 生成唯一ID
		static std::mt19937_64 s_RandomEngine([] {
#ifdef AYIN_DEBUG
			return std::mt19937_64(42);          // 固定种子，调试时可复现
#else
			static std::random_device rd;
			return std::mt19937_64(rd());
#endif
		}());
		static std::uniform_int_distribution<uint64_t> s_UniformDistribution; // 均匀分布在[0, 2 ^ 64 - 1] 区间。对 UUID 这正好是需要的——64 位全随机，碰撞概率可忽略。

		return s_UniformDistribution(s_RandomEngine);

	}

	uint64_t GetEntityUUID(Ayin::Entity entity) {

		if (!entity || !entity.HasComponents<Ayin::IDComponent>()) {
			return 0;
		}

		return entity.GetComponents<Ayin::IDComponent>().ID;

	}

	void ApplyWorldTransform(Ayin::Scene& scene, Ayin::Entity entity, const glm::mat4& worldTransform) {

		if (!entity || !entity.HasComponents<Ayin::TransformComponent>()) {
			return;
		}

		glm::mat4 localTransform = worldTransform;
		Ayin::Entity parent = scene.GetParent(entity);
		if (parent) {
			localTransform = glm::inverse(scene.CalculateWorldTransform(parent)) * worldTransform;
		}

		auto&& [position, rotation, scale] = Ayin::Math::DecomposeTransform(localTransform);
		Ayin::TransformComponent& transform = entity.GetComponents<Ayin::TransformComponent>();
		transform.Position = position;
		transform.Rotation = glm::degrees(rotation);
		transform.Scale = scale;

	}

	void DrawSpriteEntity(Ayin::Scene& scene, Ayin::Entity entity, Ayin::SpriteRendererComponent& sprite) {

		glm::mat4 worldTransform = scene.CalculateWorldTransform(entity);
		auto&& [position, rotation, scale] = Ayin::Math::DecomposeTransform(worldTransform);

		if (sprite.Texture2D.get() == nullptr)
			Ayin::Renderer2D::DrawQuad(position, glm::degrees(rotation), scale, sprite.Color);
		else
			Ayin::Renderer2D::DrawQuad(position, glm::degrees(rotation), scale, sprite.Texture2D);

	}

}

namespace Ayin{

	Entity Scene::CreateEntityWithUUID(uint64_t uuid) {

		Entity entity{ this };
		m_Registry.emplace<IDComponent>(entity, GenerateUniqueUUID(uuid));

		return entity;

	}

	uint64_t Scene::GenerateUniqueUUID(uint64_t preferredUUID) {

		if (preferredUUID != 0 && IsUUIDAvailable(preferredUUID)) {
			return preferredUUID;
		}

		uint64_t uuid = 0;
		do {
			uuid = GenerateUUID();
		} while (uuid == 0 || !IsUUIDAvailable(uuid));

		return uuid;

	}

	bool Scene::IsUUIDAvailable(uint64_t uuid, entt::entity ignoredEntity) {

		if (uuid == 0) {
			return false;
		}

		auto&& view = m_Registry.view<IDComponent>();
		for (auto&& [entity, id] : view.each()) {
			if (entity != ignoredEntity && id.ID == uuid) {
				return false;
			}
		}

		return true;

	}

	bool Scene::IsValidEntity(Entity entity) const {

		return entity && entity.m_Scene == this && m_Registry.valid(entity.m_EntityHandle);

	}

	Entity Scene::CreateEntity(const std::string& name) {

		std::string entityName = name;
		if (entityName == "Entity") {
			static int i = 0;
			entityName = fmt::format("Entity_{}", i++);
		}

		// 创建实体
		Entity entity = CreateEntityWithUUID();

		// 为实体添加常规组件
		m_Registry.emplace<TagComponent>(entity, entityName);
		m_Registry.emplace<TransformComponent>(entity);

		return entity;

	};

	Entity Scene::CreateAttachmentEntity(Entity owner) {

		if (!IsValidEntity(owner) || !owner.HasComponents<IDComponent>()) {
			return {};
		}

		Entity entity = CreateEntityWithUUID();
		m_Registry.emplace<AttachmentComponent>(entity, owner.GetComponents<IDComponent>().ID);
		m_Registry.emplace<HiddenEntityComponent>(entity);

		return entity;

	};

	Entity Scene::FindEntityByUUID(uint64_t uuid) {

		if (uuid == 0) {
			return {};
		}

		auto&& view = m_Registry.view<IDComponent>();
		for (auto&& [entity, id] : view.each()) {
			if (id.ID == uuid) {
				return Entity{ entity, this };
			}
		}

		return {};

	};

	std::vector<Entity> Scene::GetAttachmentEntities(Entity owner) {

		std::vector<Entity> attachments;
		if (!IsValidEntity(owner) || !owner.HasComponents<IDComponent>()) {
			return attachments;
		}

		uint64_t ownerID = owner.GetComponents<IDComponent>().ID;
		auto&& attachmentView = m_Registry.view<AttachmentComponent>();
		for (auto&& [entity, attachment] : attachmentView.each()) {
			if (attachment.OwnerID == ownerID) {
				attachments.emplace_back(entity, this);
			}
		}

		return attachments;

	};

	void Scene::SetParent(Entity child, Entity parent, bool keepWorldTransform) {

		if (!IsValidEntity(child) || !IsValidEntity(parent) || child == parent) {
			return;
		}
		if (child.HasComponents<AttachmentComponent>() || parent.HasComponents<AttachmentComponent>()) {
			return;
		}

		uint64_t childID = GetEntityUUID(child);
		uint64_t parentID = GetEntityUUID(parent);
		if (childID == 0 || parentID == 0) {
			return;
		}

		glm::mat4 worldTransform = keepWorldTransform ? CalculateWorldTransform(child) : glm::mat4{ 1.0f };

		// 避免环状父子关系
		Entity ancestor = parent;
		while (ancestor) {
			if (GetEntityUUID(ancestor) == childID) {
				return;
			}
			ancestor = GetParent(ancestor);
		}

		Unparent(child, false);

		auto& childRelationship = child.AddComponent<RelationshipComponent>();
		childRelationship.ParentID = parentID;

		auto& parentRelationship = parent.AddComponent<RelationshipComponent>();
		if (std::find(parentRelationship.ChildrenIDs.begin(), parentRelationship.ChildrenIDs.end(), childID) == parentRelationship.ChildrenIDs.end()) {
			parentRelationship.ChildrenIDs.push_back(childID);
		}

		if (keepWorldTransform) {
			ApplyWorldTransform(*this, child, worldTransform);
		}

	};

	void Scene::Unparent(Entity child, bool keepWorldTransform) {

		if (!IsValidEntity(child) || !child.HasComponents<RelationshipComponent>()) {
			return;
		}

		uint64_t childID = GetEntityUUID(child);
		if (childID == 0) {
			return;
		}

		glm::mat4 worldTransform = keepWorldTransform ? CalculateWorldTransform(child) : glm::mat4{ 1.0f };

		auto& childRelationship = child.GetComponents<RelationshipComponent>();
		uint64_t parentID = childRelationship.ParentID;
		if (parentID == 0) {
			return;
		}

		childRelationship.ParentID = 0;

		Entity parent = FindEntityByUUID(parentID);
		if (parent && parent.HasComponents<RelationshipComponent>()) {
			auto& parentRelationship = parent.GetComponents<RelationshipComponent>();
			parentRelationship.ChildrenIDs.erase(
				std::remove(parentRelationship.ChildrenIDs.begin(), parentRelationship.ChildrenIDs.end(), childID),
				parentRelationship.ChildrenIDs.end()
			);
		}

		if (keepWorldTransform) {
			ApplyWorldTransform(*this, child, worldTransform);
		}

	};

	std::vector<Entity> Scene::GetChildren(Entity entity) {

		std::vector<Entity> children;
		if (!IsValidEntity(entity) || !entity.HasComponents<RelationshipComponent>()) {
			return children;
		}

		const auto& relationship = entity.GetComponents<RelationshipComponent>();
		children.reserve(relationship.ChildrenIDs.size());
		for (uint64_t childID : relationship.ChildrenIDs) {
			Entity child = FindEntityByUUID(childID);
			if (child) {
				children.push_back(child);
			}
		}

		return children;

	};

	Entity Scene::GetParent(Entity entity) {

		if (!IsValidEntity(entity) || !entity.HasComponents<RelationshipComponent>()) {
			return {};
		}

		uint64_t parentID = entity.GetComponents<RelationshipComponent>().ParentID;
		return FindEntityByUUID(parentID);

	};

	glm::mat4 Scene::CalculateWorldTransform(Entity entity) {

		if (!IsValidEntity(entity) || !entity.HasComponents<TransformComponent>()) {
			return glm::mat4{ 1.0f };
		}

		glm::mat4 worldTransform = entity.GetComponents<TransformComponent>().GetLocalTransform();
		std::unordered_set<uint64_t> visited;
		visited.insert(GetEntityUUID(entity));

		Entity parent = GetParent(entity);
		while (parent && parent.HasComponents<TransformComponent>()) {
			uint64_t parentID = GetEntityUUID(parent);
			if (parentID == 0 || visited.contains(parentID)) {
				break;
			}

			visited.insert(parentID);
			worldTransform = parent.GetComponents<TransformComponent>().GetLocalTransform() * worldTransform;
			parent = GetParent(parent);
		}

		return worldTransform;

	};

	void Scene::DestroyEntity(const Entity& entity) {

		if (!IsValidEntity(entity)) {
			return;
		}

		Entity entityToDestroy = entity;
		std::vector<Entity> children = GetChildren(entityToDestroy);
		for (Entity child : children) {
			DestroyEntity(child);
		}

		std::vector<Entity> attachments = GetAttachmentEntities(entityToDestroy);
		for (Entity attachment : attachments) {
			DestroyEntity(attachment);
		}

		if (entityToDestroy.HasComponents<RelationshipComponent>()) {
			Unparent(entityToDestroy, false);
		}

		uint64_t entityID = GetEntityUUID(entityToDestroy);
		if (entityID != 0) {
			auto&& relationshipView = m_Registry.view<RelationshipComponent>();
			for (auto&& [_, relationship] : relationshipView.each()) {
				relationship.ChildrenIDs.erase(
					std::remove(relationship.ChildrenIDs.begin(), relationship.ChildrenIDs.end(), entityID),
					relationship.ChildrenIDs.end()
				);
			}
		}

		m_Registry.destroy(entityToDestroy.m_EntityHandle);

	}

	void Scene::OnUpdateRuntime(Timestep deltaTime) {
	

		//x 系统更新
		//x {
		//x 	CameraSystem::OnUpdate(m_Registry);
		//x }

		OnUpdateScripts(deltaTime);



		//? 关于没有与这些操作数匹配的 "!=" 运算符
		//! 这个错误应该来源于for语法糖展开后发现多组件view的begin()和end()类型并不相同，循环终止判断出现语法错误
		//! 
		//! 在 2020 年左右的旧版 EnTT 中，该库的设计更为“宽松”。无论一个视图（view）包含一个还是十个组件，在迭代时默认都只返回实体 ID。
		//! 然而，随着游戏引擎（例如你正在开发的引擎）变得日益复杂，EnTT 的开发者们意识到，仅返回 ID 往往效率不高。因为拿到 ID 后，你还得回到注册表重新“查找”组件（虽然 O(1) ，但仍涉及哈希查找或数组访问）。
		//! 现代 EnTT 已转向“按需输出”模式。它不再尝试预判用户的意图，而是强制要求通过 .each() 等特定方法，明确指示编译器应当如何检索数据。
		//! 
		//! 问题所在：在现代 C++ 中，EnTT 为了优化循环，为循环的 end() 引入了哨兵（Sentinel）机制。
		//! 如果你尝试像遍历普通列表那样进行迭代（例如 for (auto entity : view) ），编译器会尝试使用 != 运算符将复杂的“迭代器”类型与不同类型的“哨兵”进行比较。
		//! 由于这两个内部类型不匹配，编译器在无法确定比较规则的情况下会直接报错。
		auto&& cameraView = m_Registry.view<CameraComponent, TagComponent>();
		CameraComponent cameraComponent;
		for (auto&& [entity, camera, tag] : cameraView.each()) {

			if (tag.Name == "MainCamera") {
				cameraComponent = camera;
			}

		}
		//! 关于哨兵机制其中涉及C++20 的 Ranges 设计理念。简单来说通过结束信号，而非过去明确的结束位置来判断是否终止，这样可以少一次寻找结尾的遍历
		//除了for，也可以使用each配合lambda

		// 渲染更新
		{

			auto spriteEntities = GetEntitiesByComponents<SpriteRendererComponent, TransformComponent>();

			std::sort(spriteEntities.begin(), spriteEntities.end(), [this, &cameraComponent](Entity entity_1, Entity entity_2) -> bool {
				glm::mat4 viewMatrix = cameraComponent.Camera.GetViewMatrix();
				return (viewMatrix * CalculateWorldTransform(entity_1) * glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }).z < (viewMatrix * CalculateWorldTransform(entity_2) * glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }).z;	//简称：真的情况排在前头
				});

			Renderer2D::BeginScene(cameraComponent.Camera);

			for (Entity& entity : spriteEntities) {
				//是倒着遍历的

				DrawSpriteEntity(*this, entity, entity.GetComponents<SpriteRendererComponent>());

			}

			Renderer2D::EndScene();
		}

	};

	void Scene::OnUpdateSimulation(Timestep deltaTime, EditorCamera& editorCamera) {

		OnUpdateScripts(deltaTime);
		OnUpdateEditor(deltaTime, editorCamera);

	};


	void Scene::OnUpdateEditor(Timestep deltaTime, EditorCamera& editorCamera) {
		

		// 渲染更新
		{

			auto spriteEntities = GetEntitiesByComponents<SpriteRendererComponent, TransformComponent>();

			std::sort(spriteEntities.begin(), spriteEntities.end(), [this, &editorCamera](Entity entity_1, Entity entity_2) -> bool {
				
				glm::mat4 viewMatrix = editorCamera.GetViewMatrix();

				return  (viewMatrix * CalculateWorldTransform(entity_1) * glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }).z < (viewMatrix * CalculateWorldTransform(entity_2) * glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }).z;

				});

			Renderer2D::BeginScene(editorCamera);

			for (Entity& entity : spriteEntities) {
				//是倒着遍历的

				DrawSpriteEntity(*this, entity, entity.GetComponents<SpriteRendererComponent>());

			}

			Renderer2D::EndScene();
		}

	
	};

	void Scene::OnUpdateScripts(Timestep deltaTime) {

		// 脚本初始化（挂在实际脚本）、更新。附属脚本实体运行时会把脚本目标切回它的Owner实体。
		m_Registry.view<NativeScriptComponent>().each([=](entt::entity entity, NativeScriptComponent& nsc) {

			if (!nsc.HasScript()) {//没有绑定脚本类型
				return;
			}

			Entity scriptTarget{ entity, this };
			if (auto* attachment = m_Registry.try_get<AttachmentComponent>(entity)) {
				scriptTarget = FindEntityByUUID(attachment->OwnerID);
				if (!scriptTarget) {
					return;
				}
			}

			if (!nsc.ScriptableInstance) {//没有脚本实例
				AYIN_CORE_ASSERT(nsc.InstantiateFunction, "Script '{}' is not bound", nsc.ScriptName);
				nsc.InstantiateFunction();
				if (nsc.ScriptableInstance == nullptr) {
					return;
				}

				nsc.ScriptableInstance->m_Entity = scriptTarget;
				nsc.ScriptableInstance->OnCreate();
			}

			nsc.ScriptableInstance->OnUpdate(deltaTime);

		});

	};


	void Scene::OnViewportResize(int width, int height) {

		auto&& view = m_Registry.view<CameraComponent>();

		for (auto&& [_, cameraComponent] : view.each()) {
			cameraComponent.Camera.SetCameraSize(width, height);
		}

	};

}
