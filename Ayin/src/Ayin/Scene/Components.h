#pragma once

#include "Ayin/Core/Core.h"
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity

#include "Ayin/Renderer/Renderer2D.h"
#include "Ayin/Renderer/Texture.h"

namespace Ayin {


	struct TagComponent{
	
		std::string Name{"Entity"};

		TagComponent() = default;
		TagComponent(const std::string& name)
			:Name{ name }
		{};
		TagComponent(const TagComponent& tagComponent) = default;
	};

	struct TransformComponent {
		
		glm::vec3 Position	{ 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation	{ 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale		{ 1.0f, 1.0f, 1.0f };


		operator glm::mat4() {
		
			glm::mat4 transform{ 1.0f };

			transform = glm::translate(transform, Position);
			transform = glm::scale(transform, Scale);

			glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			return transform = transform * pitch * yaw * roll;

		};

		TransformComponent() = default;
		TransformComponent(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
			: Position{ position }, Rotation{ rotation }, Scale{ scale }
		{};
		TransformComponent(const TransformComponent& transformComponent) = default;
	};



	struct SpriteRendererComponent {
	
		//ToDo 调整一下
		Ref<Texture2D> Texture2D = nullptr;
		glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};
	
		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent& spriteRendererComponent) = default;
	};

}