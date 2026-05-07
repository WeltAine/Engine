#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/Entity.h"
#include "Ayin/Scene/Components.h"

#include <entt/entt.hpp>

namespace Ayin {

	class AYIN_API CameraSystem {

	public:
		static void OnUpdate(Ref<Scene> scene) {

			auto&& group = scene->GetEntitiesByComponents<CameraComponent>();

			for (auto& entity: group) {

				TransformComponent& transform = entity.GetComponents<TransformComponent>();
				CameraComponent& camera = entity.GetComponents<CameraComponent>();

				camera.Camera.SetViewMatrix(transform.Position, transform.Rotation);

			}


		}

	};


}
