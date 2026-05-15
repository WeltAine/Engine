#include <AyinPch.h>

#include <glm/glm.hpp>

#include <random>

#include "Ayin/Scene/Scene.h"
#include "Ayin/Scene/Entity.h"

#include "Ayin/Scene/Components.h"
#include "Ayin/Scene/Systems.h"

#include "Ayin/Renderer/Renderer2D.h"

namespace Ayin{

	Entity Scene::CreateEntity(const std::string& name) {
	
		if (name == "Entity") {
			static int i = 0;
			const_cast<std::string&>(name) = fmt::format("Entity_{}", i++);
		}

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

		// 创建实体
		Entity entity{this};

		// 为实体添加常规组件
		m_Registry.emplace<IDComponent>(entity, s_UniformDistribution(s_RandomEngine));
		m_Registry.emplace<TagComponent>(entity, name);
		m_Registry.emplace<TransformComponent>(entity);

		return entity;

	};

	void Scene::DestroyEntity(const Entity& entity) {

		m_Registry.destroy(entity.m_EntityHandle);

	}

	void Scene::OnUpdate(Timestep deltaTime) {
	

		//x 系统更新
		//x {
		//x 	CameraSystem::OnUpdate(m_Registry);
		//x }

		// 脚本更新
		{
			m_Registry.view<NativeScriptComponent>().each([=](entt::entity entity, NativeScriptComponent& nsc) {

				if (!nsc.ScriptableInstance) {
					nsc.InstantiateFunction();
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

			spriteGroup.sort<TransformComponent>([&cameraComponent](const TransformComponent& transform_1, const TransformComponent& transform_2) -> bool
					{
					    glm::mat4 ViewMatrix = cameraComponent.Camera.GetViewMatrix();
						return (ViewMatrix* glm::vec4{ transform_1.Position, 1.0f }).z < (ViewMatrix * glm::vec4{ transform_2.Position, 1.0f }).z;	//简称：真的情况排在前头
					});

			Renderer2D::BeginScene(cameraComponent.Camera);

			for (auto&& [entity, sprite, transform] : spriteGroup.each()) {
				//是倒着遍历的


				if (sprite.Texture2D.get() == nullptr)
					Renderer2D::DrawQuad(transform.Position, transform.Rotation, transform.Scale, sprite.Color);
				else
					Renderer2D::DrawQuad(transform.Position, transform.Rotation, transform.Scale, sprite.Texture2D);

			}

			Renderer2D::EndScene();
		}

	};


}
