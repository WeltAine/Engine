#include <AyinPch.h>


#include "Ayin/Scene/Systems.h"

#include "Ayin/Scene/Components.h"

#include "Ayin/Renderer/Renderer2D.h"

#include "Ayin/Math/Math.h"

#include "Ayin/Scene/SystemSchedule.h"

#include <entt/entt.hpp>


namespace Ayin::Systems {

	void CameraSystem::OnUpdate(const SystemContext& systemContext) {

		auto&& group = systemContext.Scene.GetEntitiesByComponents<CameraComponent>();

		for (auto& entity : group) {

			TransformComponent& transform = entity.GetComponents<TransformComponent>();
			CameraComponent& camera = entity.GetComponents<CameraComponent>();

			camera.Camera.SetViewMatrix(transform.Position, transform.Rotation);

		}


	}

	// -----------------------------------------------------------------------------------------------------


	void ScriptSystem::OnUpdate(const SystemContext& systemContext) {

		// 脚本初始化（挂在实际脚本）、更新
		{

			std::vector<::Ayin::Entity>&& entities = systemContext.Scene.GetEntitiesByComponents<NativeScriptComponent>();

			std::for_each(entities.begin(), entities.end(),
				[=](::entt::entity entity) {

					Entity aimEntity = Entity{ entity, &systemContext.Scene };
					NativeScriptComponent& nsc = aimEntity.GetComponents<NativeScriptComponent>();

					if (!nsc.HasScript()) {//没有绑定脚本类型
						return;
					}

					if (!nsc.ScriptableInstance) {//没有脚本实例
						nsc.Instantiate();
						if (nsc.ScriptableInstance == nullptr) {
							return;
						}
						nsc.ActiveScript(aimEntity);
					}

					nsc.Update(systemContext.DeltaTime);

				});

			//! 感觉没有以前的写法好，现在的写法看起来相比起来，感觉一点也不 ECS

			//m_Registry.view<NativeScriptComponent>().each([=](entt::entity entity, NativeScriptComponent& nsc) {

			//	if (!nsc.HasScript()) {//没有绑定脚本类型
			//		return;
			//	}

			//	if (!nsc.ScriptableInstance) {//没有脚本实例
			//		AYIN_CORE_ASSERT(nsc.InstantiateFunction, "Script '{}' is not bound", nsc.ScriptName);
			//		nsc.InstantiateFunction();
			//		if (nsc.ScriptableInstance == nullptr) {
			//			return;
			//		}
			//		nsc.ScriptableInstance->m_Entity = Entity{ entity, this };
			//		nsc.ScriptableInstance->OnCreate();
			//	}

			//	nsc.ScriptableInstance->OnUpdate(deltaTime);

			//	});


		};


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

};

