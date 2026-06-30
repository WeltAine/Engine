#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/Scene.h"
#include "Ayin/Scene/Entity.h"

namespace Ayin {

	namespace Systems {

		// -----------------------------------------------------------------------------------------------------
		class CameraSystem {

		public:
			static void OnUpdate(Ref<Scene> scene);

		};

	};

};
