#include <AyinPch.h>


#include "Ayin/System/Systems.h"

#include "Ayin/Scene/Components.h"

#include "Ayin/Renderer/Renderer2D.h"

#include "Ayin/Math/Math.h"

#include "Ayin/System/SystemSchedule.h"

#include "Ayin/Core/UUID.h"

#include "Ayin/Scene/ScriptRegistry.h"

#include <entt/entt.hpp>


namespace Ayin{

	void CameraSystem::OnUpdate(const SystemContext& systemContext) {

		auto&& group = systemContext.Scene.GetEntitiesByComponents<CameraComponent>();

		for (auto& entity : group) {

			TransformComponent& transform = entity.GetComponents<TransformComponent>();
			CameraComponent& camera = entity.GetComponents<CameraComponent>();

			camera.Camera.SetViewMatrix(transform.Position, transform.Rotation);

		}


	}

	// -----------------------------------------------------------------------------------------------------


	void ScriptSystem::OnPreUpdate(const SystemContext& systemContext) {

		//! 处理上一帧的删除和 bind

		auto func = [](entt::entity, NativeScriptComponent& nsc) -> void {

				//! 那些可能调用到即将销毁的 script 的阶段不要在该阶段运行。
				//! 顺序固定为：先释放当前 head，再提交最后一次 Bind 请求，最后实例化新 head。
				if (nsc.m_ReleaseHeadNextFrame) {
					nsc.StopScript();
				}

				if (nsc.HasPendingBind())
					nsc.CommitPendingBind();

				nsc.InstantiateHead();		// 内有 前置 Bound 状态要求

			};


		systemContext.Scene.Each<NativeScriptComponent>(func);



	};

	void ScriptSystem::OnUpdate(const SystemContext& systemContext) {

		auto func = [&scene = systemContext.Scene, deltaTime = systemContext.DeltaTime](entt::entity entity, NativeScriptComponent& nsc) -> void {

			switch (nsc.GetScriptLifecycleState()) {

				case(NativeScriptComponent::ScriptLifecycleState::Unbound):
					break;

				case(NativeScriptComponent::ScriptLifecycleState::Instantiated):

					if (nsc.ScriptableInstance) {
						nsc.ScriptableInstance->SetEntity(Entity{ entity, &scene });

						// 反序列化
						const bool noException = ScriptRegistry::DeserializeScriptByScriptName(nsc, nsc.ScriptName, nsc.ScriptData.str);
						if(!noException && nsc.ScriptData.str != NativeScriptComponent::NullScriptData && !nsc.ScriptData.str.empty())
							AYIN_CORE_WARN(" {0} script deserialize failure", nsc.ScriptName);
						nsc.ScriptData = NativeScriptComponent::NullScriptData;

						// Create 流程
						nsc.ScriptableInstance->OnCreate();

						//! Bind/UnBind 在 OnCreate 中只会登记下一帧的待处理变化，当前 head 仍完成 OnCreate 并进入 Active。
						nsc.m_State = NativeScriptComponent::ScriptLifecycleState::Active;
					}
					else {
						AYIN_CORE_ERROR("Script Instance is nullptr, Lifecycle exception");
						AYIN_CORE_ASSERT(false);

						break;
					}

					[[fallthrough]];	//显式贯穿（表明这里是故意的，初始化，OnCreate OnUpdate 在同一帧一起执行）

				case(NativeScriptComponent::ScriptLifecycleState::Active):

					if(nsc.ScriptableInstance)
						nsc.ScriptableInstance->OnUpdate(deltaTime);

					break;

				default: break;

			}

			};

		systemContext.Scene.Each<NativeScriptComponent>(func);

	}


	void ScriptSystem::StopAllScript(Scene& scene) {
		
		std::vector<::Ayin::Entity>&& entities = scene.GetEntitiesByComponents<NativeScriptComponent>();

		std::ranges::for_each(entities, 
			[](Entity entity) {
				entity.GetComponents<NativeScriptComponent>().StopScript();
			});

	};


	// -----------------------------------------------------------------------------------------------------


	void RenderSystem::OnUpdate(const SystemContext& systemContext) {

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
		std::vector <::Ayin::Entity>&& cameraEntities = systemContext.Scene.GetEntitiesByComponents<CameraComponent, TagComponent>();
		CameraComponent mainCameraComponent;
		for (::Ayin::Entity entity : cameraEntities) {

			TagComponent& tag = entity.GetComponents<TagComponent>();

			if (tag.Name == "MainCamera") {
				mainCameraComponent = entity.GetComponents<CameraComponent>();
			}

		}
		//! 关于哨兵机制其中涉及C++20 的 Ranges 设计理念。简单来说通过结束信号，而非过去明确的结束位置来判断是否终止，这样可以少一次寻找结尾的遍历
		//除了for，也可以使用each配合lambda

		// 渲染更新
		{
			std::vector<Entity>&& renderEntities = systemContext.Scene.GetEntitiesByComponents<SpriteRendererComponent, TransformComponent>();

			//! 从投影矩阵的分析中，确实可以肯定，相机试图看到的确实是 view 空间下位于 -z 轴的那一部分内容（从平移的部分可以看出）。
			//! 但是在缩放那部分中有对z值取反是因为
			//! OpenGL 默认 NDC 深度是：
			//!	near → - 1
			//!	far → + 1
			//x 之前关于 group 的遍历顺序结论是错的（估计当时是我比较lambda写错了，之后改了却又没删那段注释）
			std::ranges::sort(renderEntities, [this, &mainCameraComponent, &systemContext](const ::entt::entity& entity_1, const ::entt::entity& entity_2) -> bool
				{
					glm::mat4 viewMatrix = mainCameraComponent.Camera.GetViewMatrix();
					Entity _entity_1{ entity_1, &systemContext.Scene };
					Entity _entity_2{ entity_2, &systemContext.Scene };
					return (viewMatrix * _entity_1.GetWorldTransform())[3][2] < (viewMatrix * _entity_2.GetWorldTransform())[3][2];	//简称：真的情况排在前头
				});

			Renderer2D::BeginScene(mainCameraComponent.Camera);

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

	}




	// -----------------------------------------------------------------------------------------------------

	void DestroySystem::OnPreUpdate(const SystemContext& systemContext) {

		auto func =
			[&scene = systemContext.Scene](entt::entity entity, DestroyComponent& destroyComponent) -> void {

			if (destroyComponent.IsDestroyEntity) {
				UUIDGenerator::NullifyUUID(scene.m_Registry.get<IDComponent>(entity).ID);
				if (scene.m_Registry.valid(entity)) {
					scene.m_Registry.destroy(entity);
					//! EnTT 文档明确允许在 view 迭代时删除“当前实体”或其当前组件
				};

				return;
			}

			for (::entt::id_type componentId : destroyComponent.DestoryComponents) {

				if (componentId != IDComponent::ComponentStorageID()
					&& componentId != TransformComponent::ComponentStorageID()
					&& componentId != TagComponent::ComponentStorageID()) {

					auto storage = scene.m_Registry.storage(componentId);
					if (storage) storage->remove(entity);

				}
			}

			scene.m_Registry.storage(DestroyComponent::ComponentStorageID())->remove(entity);


			};


		systemContext.Scene.Each<DestroyComponent>(func);

	};


};
