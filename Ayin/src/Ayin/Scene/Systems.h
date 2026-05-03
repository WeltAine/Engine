#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/Entity.h"
#include "Ayin/Scene/Components.h"

#include <entt/entt.hpp>

namespace Ayin {

	class AYIN_API CameraSystem {

	public:
		static void OnUpdate(entt::registry& registry) {

			auto&& group = registry.view<CameraComponent>();

			for (auto& entity: group) {

				TransformComponent& transform = registry.get<TransformComponent>(entity);
				CameraComponent& camera = registry.get<CameraComponent>(entity);

				camera.Camera.SetViewMatrix(transform.Position, transform.Rotation);

			}


		}

	};


}