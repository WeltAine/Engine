#include <AyinPch.h>

#include <glm/glm.hpp>

#include <random>

#include "Ayin/Scene/Scene.h"
#include "Ayin/Scene/Entity.h"

#include "Ayin/Scene/Components.h"
#include "Ayin/System/Systems.h"
#include "Ayin/System/SystemSchedule.h"

#include "Ayin/Renderer/Renderer2D.h"

#include "Ayin/Math/Math.h"

#include "Ayin/Core/UUID.h"


namespace Ayin{

	// ----------------------------------------------------------------------------

	Entity Scene::CreateUUIDEntity() {
	
		Entity entity{ this };

		m_Registry.emplace<IDComponent>(entity, UUIDGenerator::GenerateUniqueID());

		return entity;
	
	};


	Entity Scene::CreateEntity(const std::string& name) {
	
		if (name == "Entity") {
			static int i = 0;
			const_cast<std::string&>(name) = fmt::format("Entity_{}", i++);
		}


		// 创建实体（仅带UUID）
		Entity entity = CreateUUIDEntity();

		// 为实体添加常规组件
		m_Registry.emplace<TagComponent>(entity, name);
		m_Registry.emplace<TransformComponent>(entity);

		return entity;

	};

	void Scene::DestroyEntity(Entity& entity) {

		if (!entity || entity.m_Scene != this)
			return;

		// 关系处理
		UnParent(entity, false);

		auto DFS = [&scene = *this](this auto& self, Entity& entity) -> void {

			if (!entity.HasComponents<RelationShipComponent>())
				return;	// 递归底线

			RelationShipComponent& relationShipComponent = entity.GetComponents<RelationShipComponent>();

			// 删除本节点
			entity.AddComponent<Ayin::DestroyComponent>().IsDestroyEntity = true;

			// 递归处理子节点
			auto childrenUUID = relationShipComponent.ChildrenUUID;	//! 因为 DestroyEntity 会对组件中的数组进行修改，所以直接对着组件数据迭代会发生边修改边迭代的情况
			for (auto UUID : childrenUUID) {
				Entity child = scene.FindEntityByUUID(UUID);

				if(child && child.m_Scene == &scene)	// 防止脏数据
					self(child);
			}

			};

		DFS(entity);
	}

	void Scene::DestroyComponent(Entity& entity, ::entt::id_type componentId) {

		if (!entity || entity.m_Scene != this)
			return;

		entity.AddComponent<Ayin::DestroyComponent>().DestoryComponents.insert(componentId);


	};


	// ----------------------------父子关系接口------------------------------------
	void Scene::SetParent(Entity& child, Entity& parent, bool keepWorldTransform) {
	
		//Todo 或许该尝试支持跨场景
		
		// ---------------------安全守卫----------------------

		if (child.m_Scene != this || parent.m_Scene != this) return;

		uint64_t childUUID = child.GetComponents<IDComponent>().ID;
		uint64_t parentUUID = parent.GetComponents<IDComponent>().ID;

		// UUID 有效并且不充许跨场景设置
		if (childUUID == parentUUID || childUUID == 0 || parentUUID == 0 ) return;

		// 用 Add 确保一定存在 RelationShipComponent，并且提前，确保 DFS 不会崩溃
		RelationShipComponent& childRelationShipComponent = child.AddComponent<RelationShipComponent>();
		RelationShipComponent& parentRelationShipComponent = parent.AddComponent<RelationShipComponent>();


		if (IsDescendant(parent, child)) return;


		// ---------------------关系清理----------------------

		glm::mat4 oldChildWorldMatrix = child.GetWorldTransform();

		// 清理旧旧关系

		Entity child_parent = GetParent(child);
		if (child_parent) {
			
			std::vector<uint64_t>& childrenUUID = child_parent.GetComponents<RelationShipComponent>().ChildrenUUID;

			std::erase(childrenUUID, childUUID);
			// std::remove 和 std::remove_if			将满足要求的元素重排到后头
			// std::erase / std::erase_if				将满足要求的元素删除
			// std::partition / std::stable_partition		将满足要求的元素重排到前头

			childRelationShipComponent.ParentUUID = 0;

		}

		// 建立新关系
		parentRelationShipComponent.ChildrenUUID.emplace_back(child.GetComponents<IDComponent>().ID);
		childRelationShipComponent.ParentUUID = parent.GetComponents<IDComponent>().ID;
		
		
		
		// -------------------坐标调整---------------------

		if (keepWorldTransform) {
			// 子实体和目标父实体的世界变换矩阵
			glm::mat4 parentWorldMatrix = parent.GetWorldTransform();

			// 得到子实体在新父实体下的局部变换矩阵
			glm::mat4 childLocalMatrix = glm::inverse(parentWorldMatrix) * oldChildWorldMatrix;

			// 拆解新局部矩阵与写回数据
			TransformComponent& transform = child.GetComponents<TransformComponent>();

			auto [position, rotation, scale] = Math::DecomposeTransform(childLocalMatrix);
			transform.Position = position;
			transform.Rotation = rotation;
			transform.Scale = scale;

		}

	};

	void Scene::UnParent(Entity& child, bool keepWorldTransform) {

		// ---------------------安全守卫----------------------

		uint64_t childUUID = child.GetComponents<IDComponent>().ID;

		if (childUUID == 0 || child.m_Scene != this) return;

		glm::mat4 oldChildWorldMatrix = child.GetWorldTransform();



		// ---------------------关系清理----------------------

		// 清理旧旧关系
		RelationShipComponent& childRelationShipComponent = child.AddComponent<RelationShipComponent>();

		Entity child_parent = GetParent(child);
		if (child_parent) {

			std::vector<uint64_t>& childrenUUID = child_parent.GetComponents<RelationShipComponent>().ChildrenUUID;

			std::erase(childrenUUID, childUUID);
			// std::remove 和 std::remove_if			将满足要求的元素重排到后头
			// std::erase / std::erase_if				将满足要求的元素删除
			// std::partition / std::stable_partition		将满足要求的元素重排到前头

			childRelationShipComponent.ParentUUID = 0;

		}

		// 坐标调整

		if (keepWorldTransform) {
			
			TransformComponent& transform = child.GetComponents<TransformComponent>();

			auto [position, rotation, scale] = Math::DecomposeTransform(oldChildWorldMatrix);
			transform.Position = position;
			transform.Rotation = rotation;
			transform.Scale = scale;

		}


	};



	Entity Scene::GetParent(const Entity& child) const {
	
		if (!child.HasComponents<RelationShipComponent>())	//没有关系组件
			return Entity{};

		uint64_t parentUUID = child.GetComponents<RelationShipComponent>().ParentUUID;

		if (!UUIDGenerator::IsUUIDAlive(parentUUID))				//有对应关系组件但不是有效父节点
			return Entity{};

		// 该方法内部做了 entity 所属检测
		return FindEntityByUUID(parentUUID);

	};

	std::vector<Entity> Scene::GetChilds(const Entity& parent) const {
	
		if (!parent.HasComponents<RelationShipComponent>())	//没有关系组件
			return std::vector<Entity>{};

		RelationShipComponent& relationShipComponent = parent.GetComponents<RelationShipComponent>();

		//! 当实体不属于场景时会返回一系列空实体。

		std::vector<Entity> childs;
		for (uint64_t UUID : relationShipComponent.ChildrenUUID) {
		
			if(UUIDGenerator::IsUUIDAlive(UUID))
				childs.emplace_back(FindEntityByUUID(UUID));

		}

		return childs;

	};

	bool Scene::IsDescendant(const Entity& entity, const Entity& parent){
	
		if (!entity || !parent || entity.m_Scene != parent.m_Scene) return false;

		uint64_t entityUUID = entity.GetComponents<IDComponent>().ID;

		if (!parent.HasComponents<RelationShipComponent>())
			return false;

#pragma region 递归 lambda 笔记
		// 深度优先遍历，避免回环（parent 是 child 的一个子实体）
		// 递归 lambda 笔记：同一个 DFS 可以用几种写法实现。
		//
		//! C++ lambda 是匿名函数对象，lambda 体内没有天然的“自身名字”，
		//! 所以不能像普通函数那样直接在函数体里写 `DFS(...)` 调回自己。
		//! 下面这些写法做的是同一件事：从 child 开始向下遍历子树，
		//! 如果在任意后代中找到了 parentUUID，说明把 child 挂到 parent 下会形成环。
		//
		//! 写法 1：std::function 提前声明，给 lambda 一个可捕获的名字。
		//
		//x    std::function<bool(const Entity&)> DFS;
		//x    DFS = [this, parentUUID, &DFS](const Entity& node) -> bool {
		//x        auto& relation = node.GetComponents<RelationShipComponent>();
		//x
		//x        if (std::ranges::find(relation.ChildrenUUID, parentUUID) != relation.ChildrenUUID.end())
		//x            return true;
		//x
		//x        for (uint64_t uuid : relation.ChildrenUUID) {
		//x            Entity child = this->FindEntityByUUID(uuid);
		//x            if (child && DFS(child)) return true;
		//x        }
		//x
		//x        return false;
		//x    };
		//x
		//x    if (DFS(child)) return;
		//x
		//! 优点：递归调用处最像普通函数，`DFS(child)` 很自然。
		//! 缺点：`std::function` 是类型擦除包装，可能有额外开销；而且这里捕获了局部
		//! `DFS` 的引用，如果这个 lambda 被保存到外部再调用，就会产生悬垂引用。
		//x
		//! 写法 2：C++14 起，显式把 lambda 自己作为参数传进去。
		//x
		//x    auto DFS = [this, parentUUID](auto&& self, const Entity& node) -> bool {
		//x        auto& relation = node.GetComponents<RelationShipComponent>();
		//x
		//x        if (std::ranges::find(relation.ChildrenUUID, parentUUID) != relation.ChildrenUUID.end())
		//x            return true;
		//x
		//x        for (uint64_t uuid : relation.ChildrenUUID) {
		//x            Entity child = this->FindEntityByUUID(uuid);
		//x            if (child && self(self, child)) return true;
		//x        }
		//x
		//x        return false;
		//x    };
		//x
		//x    if (DFS(DFS, child)) return;
		//x
		//! 优点：不需要 `std::function`，没有类型擦除，也没有捕获局部函数包装器引用的问题。
		//! 缺点：入口和递归点都要写 `self(self, ...)`，读起来像“把自己传给自己”。
		//x
		//! 写法 3：C++23 explicit object parameter / deducing this，也就是当前写法。
		//x
		//x    auto DFS = [this, parentUUID](this auto& self, const Entity& node) -> bool {
		//x        auto& relation = node.GetComponents<RelationShipComponent>();
		//x
		//x        if (std::ranges::find(relation.ChildrenUUID, parentUUID) != relation.ChildrenUUID.end())
		//x            return true;
		//x
		//x        for (uint64_t uuid : relation.ChildrenUUID) {
		//x            Entity child = this->FindEntityByUUID(uuid);
		//x            if (child && self(child)) return true;
		//x        }
		//x
		//x        return false;
		//x    };
		//x
		//x    if (DFS(child)) return;
		//x
		//! `this auto& self` 把 lambda 对象自己作为显式对象参数暴露出来，
		//! 所以递归时可以直接 `self(child)`，既没有 `std::function`，也不用 `self(self, child)`。
		//! 这里用 `auto&` 是可以的，因为 DFS 保存到局部变量后，表达式 `DFS` 是左值；
		//! 如果要直接调用一个临时 lambda，例如 `[](this auto&& self, ...) { ... }(...)`，
		//! 就应使用 `this auto&& self` 来同时接住左值和右值 lambda 对象。
		//x
		//! 写法 4：Y Combinator / fixed-point combinator，把“传自己给自己”封装成工具。
		//x
		//x    template<typename F>
		//x    struct Fix {
		//x        F f;
		//x
		//x        template<typename... Args>
		//x        decltype(auto) operator()(Args&&... args) {
		//x            return f(*this, std::forward<Args>(args)...);
		//x        }
		//x    };
		//x
		//x    auto DFS = Fix{[this, parentUUID](auto& self, const Entity& node) -> bool {
		//x        auto& relation = node.GetComponents<RelationShipComponent>();
		//x
		//x        if (std::ranges::find(relation.ChildrenUUID, parentUUID) != relation.ChildrenUUID.end())
		//x            return true;
		//x
		//x        for (uint64_t uuid : relation.ChildrenUUID) {
		//x            Entity child = this->FindEntityByUUID(uuid);
		//x            if (child && self(child)) return true;
		//x        }
		//x
		//x        return false;
		//x    }};
		//x
		//x    if (DFS(child)) return;
		//x
		//! 优点：调用处也能保持 `DFS(child)`，并且不需要 `std::function`。
		//! 缺点：为了一个局部 DFS 引入组合子模板，抽象成本明显高于收益；
		//! 在这里 C++23 的 `this auto& self` 更直接、更贴合“局部递归 lambda”的需求。
#pragma endregion

		// DFS
		Scene* scene = parent.m_Scene;

		auto DFS = [scene, entityUUID](this auto& self, const Entity& _parent) -> bool {

			if (!_parent) return false;

				RelationShipComponent& relationShipComponent = _parent.GetComponents<RelationShipComponent>();
				if (std::ranges::find(relationShipComponent.ChildrenUUID, entityUUID) != relationShipComponent.ChildrenUUID.end())
					return true;

				for (auto UUID : relationShipComponent.ChildrenUUID) {
 
					if (self(scene->FindEntityByUUID(UUID))) return true;

				}

				return false;

			};

		return DFS(parent);

	};



	// ----------------------------------------------------------------------------


	void Scene::OnViewportResize(int width, int height) {

		auto&& view = m_Registry.view<CameraComponent>();

		for (auto&& [_, cameraComponent] : view.each()) {
			cameraComponent.Camera.SetCameraSize(width, height);
		}

	};


	// ----------------------------------------------------------------------------

	Entity Scene::FindEntityByUUID(uint64_t UUID) const {

		auto UUIDView = m_Registry.view<IDComponent>();

		for (auto&& [entity, IDComponent] : UUIDView.each()) {

			if (IDComponent.ID == UUID)
				return Entity{ entity, const_cast<Scene*>(this) };

		}

		return Entity{};//不存在或者不属于这个Scene

	};


	// ----------------------------------------------------------------------------

	Scene::~Scene() {
	
		ScriptSystem::StopAllScript(*this);

	};

}
