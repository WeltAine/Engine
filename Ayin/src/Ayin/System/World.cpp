#include <AyinPch.h>

#include "Ayin/System/World.h"

#include "Ayin/System/SystemSchedule.h"

namespace Ayin {
	
	World::World(Ref<Scene> scene, const SystemPipeline& systemPipeline)
		:m_ActiveScene{ scene }, m_systemSchedule{systemPipeline.CreateSchedule()}
	{};

	bool World::Begin(SceneMode mode) {

		if (mode == SceneMode::None) {
			AYIN_CORE_WARN("Invalid SceneMode.");
			return false;
		}

		m_CurrentMode = mode;
		return true;

	};
	bool World::Update(Timestep deltaTime) {

		if (m_ActiveScene == nullptr || m_CurrentMode == SceneMode::None) {
			AYIN_CORE_WARN("Invalid SceneMode or Scene.");
			return false;
		}

		SystemContext context{ 
			.Scene{*m_ActiveScene},
			.DeltaTime{deltaTime},
			.Mode{m_CurrentMode},
			.Phase{SystemPhase::None}
		};

		m_systemSchedule.Run(context);

		return true;

	};

	bool World::End() {

		if (m_CurrentMode == SceneMode::None) {
			AYIN_CORE_WARN("Invalid SceneMode.");
			return false;
		}

		m_CurrentMode = SceneMode::None;
		return true;

	};



};
