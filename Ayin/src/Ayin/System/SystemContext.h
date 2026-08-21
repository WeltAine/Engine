#pragma once

#include "Ayin/Core/Timestep.h"

#include "Ayin/Scene/SceneMode.h"
#include "Ayin/System/SystemTypes.h"


namespace Ayin {

	class Scene;
	class EditorCamera;


	struct SystemContext {

		Scene& Scene;									// 当前更新的场景

		Timestep DeltaTime = {};						// 时间间隔

		SceneMode Mode = SceneMode::None;				// 场景运行模式

		SystemPhase Phase = SystemPhase::None;			// 当前阶段

		EditorCamera* EditorView = nullptr;

	};


};
