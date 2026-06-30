#include <AyinPch.h>

#include <glm/glm.hpp>

#include <random>

#include "Ayin/Scene/Scene.h"
#include "Ayin/Scene/Entity.h"

#include "Ayin/Scene/Components.h"
#include "Ayin/Scene/Systems.h"

#include "Ayin/Renderer/Renderer2D.h"

#include "Ayin/Math/Math.h"

namespace Ayin {

	// 全局ID
	static std::unordered_set<uint64_t> s_GlobalUniqueIDSet{ 0 };

	namespace Utils {

		bool IsUUIDUnused(uint64_t globalUniqueID) {

			return !s_GlobalUniqueIDSet.contains(globalUniqueID);

		};

		bool IsUUIDAlive(uint64_t globalUniqueID) {
			
			return s_GlobalUniqueIDSet.contains(globalUniqueID) && globalUniqueID != 0;
		
		}

		uint64_t GenerateUniqueID() {

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

			uint64_t globalUniqueID = 0;

			while (!IsUUIDUnused(globalUniqueID)) {
				globalUniqueID = s_UniformDistribution(s_RandomEngine);
			}

			s_GlobalUniqueIDSet.insert(globalUniqueID);

			return globalUniqueID;

		};

	}

}


namespace Ayin{

	// ----------------------------------------------------------------------------

	Entity Scene::CreateUUIDEntity() {
	
		Entity entity{ this };

		m_Registry.emplace<IDComponent>(entity, Utils::GenerateUniqueID());

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

		SubmitEntityDestroy(entity);

	}

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

		if (!Utils::IsUUIDAlive(parentUUID))				//有对应关系组件但不是有效父节点
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
		
			if(Utils::IsUUIDAlive(UUID))
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

	void Scene::OnUpdateRuntime(Timestep deltaTime) {
	
		//x 系统更新
		//x {
		//x 	CameraSystem::OnUpdate(m_Registry);
		//x }

		// 延迟删除
		FlushDestroyedEntities();

		// 脚本初始化（挂在实际脚本）、更新
		{
			m_Registry.view<NativeScriptComponent>().each([=](entt::entity entity, NativeScriptComponent& nsc) {

				if (!nsc.HasScript()) {//没有绑定脚本类型
					return;
				}

				if (!nsc.ScriptableInstance) {//没有脚本实例
					AYIN_CORE_ASSERT(nsc.InstantiateFunction, "Script '{}' is not bound", nsc.ScriptName);
					nsc.InstantiateFunction();
					if (nsc.ScriptableInstance == nullptr) {
						return;
					}
					nsc.ScriptableInstance->m_Entity = Entity{ entity, this };
					nsc.ScriptableInstance->OnCreate();
				}

				nsc.ScriptableInstance->OnUpdate(deltaTime);

				});
		}



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

			auto spriteGroup = m_Registry.group<SpriteRendererComponent>(entt::get<TransformComponent>);

			std::vector<Entity> renderEntities{};
			renderEntities.reserve(spriteGroup.size());

			for (auto&& [entity, sprit, transform] : spriteGroup.each()) {
				renderEntities.emplace_back(entity, this);
			}

			//! 从投影矩阵的分析中，确实可以肯定，相机试图看到的确实是 view 空间下位于 -z 轴的那一部分内容（从平移的部分可以看出）。
			//! 但是在缩放那部分中有对z值取反是因为
			//! OpenGL 默认 NDC 深度是：
			//!	near → - 1
			//!	far → + 1
			//x 之前关于 group 的遍历顺序结论是错的（估计当时是我比较lambda写错了，之后改了却又没删那段注释）
			std::ranges::sort(renderEntities, [this, &cameraComponent](const ::entt::entity& entity_1, const ::entt::entity& entity_2) -> bool
				{
					glm::mat4 viewMatrix = cameraComponent.Camera.GetViewMatrix();
					Entity _entity_1{ entity_1, this };
					Entity _entity_2{ entity_2, this };
					return (viewMatrix * _entity_1.GetWorldTransform())[3][2] < (viewMatrix * _entity_2.GetWorldTransform())[3][2];	//简称：真的情况排在前头
				});

			Renderer2D::BeginScene(cameraComponent.Camera);

			for (Entity& entity : renderEntities) {

				auto& sprite = entity.GetComponents<Ayin::SpriteRendererComponent>();
				//Entity parent = entity.GetParent();
				//glm::mat4 parentWorldMatrix = (parent) ? parent.GetWorldTransform() : glm::mat4{ 1.0f };
				auto [position, rotation, scale] = Ayin::Math::DecomposeTransform(entity.GetWorldTransform());

				if (sprite.Texture2D.get() == nullptr)
					Renderer2D::DrawQuad(position, rotation, scale, sprite.Color);
				else
					Renderer2D::DrawQuad(position, rotation, scale, sprite.Texture2D);

			}

			Renderer2D::EndScene();
		}

	};


	void Scene::OnUpdateEditor(Timestep deltaTime, EditorCamera& editorCamera) {
		

		// 延迟删除
		FlushDestroyedEntities();

		// 渲染更新
		{

			auto spriteGroup = m_Registry.group<SpriteRendererComponent>(entt::get<TransformComponent>);

			std::vector<Entity> renderEntities{};
			renderEntities.reserve(spriteGroup.size());

			for (auto&& [entity, sprit, transform] : spriteGroup.each()) {
				renderEntities.emplace_back(entity, this);
			}

			//! 从投影矩阵的分析中，确实可以肯定，相机试图看到的确实是 view 空间下位于 -z 轴的那一部分内容（从平移的部分可以看出）。
			//! 但是在缩放那部分中有对z值取反是因为
			//! OpenGL 默认 NDC 深度是：
			//!	near → - 1
			//!	far → + 1
			//x 之前关于 group 的遍历顺序结论是错的（估计当时是我比较lambda写错了，之后改了却又没删那段注释）
			std::ranges::sort(renderEntities, [this, &editorCamera](const ::entt::entity& entity_1, const ::entt::entity& entity_2) -> bool
				{
					glm::mat4 viewMatrix = editorCamera.GetViewMatrix();
					Entity _entity_1{ entity_1, this };
					Entity _entity_2{ entity_2, this };
					return (viewMatrix * _entity_1.GetWorldTransform())[3][2] < (viewMatrix * _entity_2.GetWorldTransform())[3][2];	//简称：真的情况排在前头
				});

			Renderer2D::BeginScene(editorCamera);

			for (Entity& entity : renderEntities) {

				auto& sprite = entity.GetComponents<Ayin::SpriteRendererComponent>();

				auto [position, rotation, scale] = Ayin::Math::DecomposeTransform(entity.GetWorldTransform());

				if (sprite.Texture2D.get() == nullptr)
					Renderer2D::DrawQuad(position, rotation, scale, sprite.Color);
				else
					Renderer2D::DrawQuad(position, rotation, scale, sprite.Texture2D);

			}

			Renderer2D::EndScene();
		}

	
	};


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
	void Scene::SubmitEntityDestroy(const Entity& entity) {
		
		if (!entity || entity.m_Scene != this)
			return;

		auto it = std::ranges::find(m_DestroyEntities, entity);

		if (it == m_DestroyEntities.end()) {
			m_DestroyEntities.insert(entity);
			return;
		}

	};

	void Scene::InternalDestroyEntity(Entity& entity) {

		if (!entity || entity.m_Scene != this) return;// 防止父子同帧入队，以及不属于本场景的有效实体

		if (entity.HasComponents<RelationShipComponent>()) {

			RelationShipComponent& relationShipComponent = entity.GetComponents<RelationShipComponent>();

			UnParent(entity, false);

			//递归处理子节点
			auto childrenUUID = relationShipComponent.ChildrenUUID;	//! 因为 DestroyEntity 会对组件中的数组进行修改，所以直接对着组件数据迭代会发生边修改边迭代的情况
			for (auto UUID : childrenUUID) {
				Entity child = FindEntityByUUID(UUID);
				InternalDestroyEntity(child);
			}

		}


		s_GlobalUniqueIDSet.erase(entity.GetComponents<IDComponent>().ID);
		if (m_Registry.valid(entity.m_EntityHandle)) {
			m_Registry.destroy(entity.m_EntityHandle);
		};
		entity = {};

	};


	void Scene::FlushDestroyedEntities() {

		//! set / unordered_set 的元素本身就是“用于排序或哈希定位的 key”
		//! key 不能被随便修改，否则哈希位置可能失效，所以遍历 unordered_set<Entity> 时拿到的通常是：const Entity&，而非 Entity&
		//! 如果返回 Entity& entity ，那意味着你可能改掉影响哈希或相等判断的字段，容器内部结构就会坏掉。
		std::ranges::for_each(m_DestroyEntities, [this](Entity entity) {
			this->InternalDestroyEntity(entity);
			});

		m_DestroyEntities.clear();

	};

}
