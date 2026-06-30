#include <AyinPch.h>

#include "Ayin/Scene/Systems.h"

#include "Ayin/Scene/Components.h"

#include <entt/entt.hpp>


namespace Ayin {

	namespace Systems {

		void CameraSystem::OnUpdate(Ref<Scene> scene) {

			auto&& group = scene->GetEntitiesByComponents<CameraComponent>();

			for (auto& entity : group) {

				TransformComponent& transform = entity.GetComponents<TransformComponent>();
				CameraComponent& camera = entity.GetComponents<CameraComponent>();

				camera.Camera.SetViewMatrix(transform.Position, transform.Rotation);

			}


		}

	};

};

