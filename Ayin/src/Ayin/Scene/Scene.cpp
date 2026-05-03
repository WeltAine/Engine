#include <AyinPch.h>

#include "Ayin/Scene/Scene.h"
#include "Ayin/Scene/Entity.h"

#include "Ayin/Scene/Components.h"

#include "Ayin/Renderer/Renderer2D.h"

namespace Ayin{

	Entity Scene::CreateEntity(const std::string& name) {
	

		Entity entity{this};

		m_Registry.emplace<TagComponent>(entity, name);
		m_Registry.emplace<TransformComponent>(entity);

		return entity;

	};

	void Scene::OnUpdate(Timestep deltaTime) {
	
		auto group = m_Registry.group<SpriteRendererComponent>(entt::get<TransformComponent>);
		for (auto entity : group) {
			//是倒着遍历的

			//ToDo 渲染排序问题

			auto&& [sprite, transform] = group.get<SpriteRendererComponent, TransformComponent>(entity);

			if (sprite.Texture2D.get() == nullptr)
				Renderer2D::DrawQuad(transform.Position, transform.Rotation, transform.Scale, sprite.Color);
			else
				Renderer2D::DrawQuad(transform.Position, transform.Rotation, transform.Scale, sprite.Texture2D);

		}
	
	};


}