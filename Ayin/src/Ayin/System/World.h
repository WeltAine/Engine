#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/Scene.h"
#include "Ayin/System/SystemSchedule.h"

#include "Ayin/System/SystemPipeline.h"

namespace Ayin {
	
	class World {
	
	private:

		Ref<Scene> m_ActiveScene;
		SystemSchedule m_systemSchedule;

		SceneMode m_CurrentMode = SceneMode::None;


	public:

		World(Ref<Scene> scene, const SystemPipeline& systemPipeline);

		// 返回 bool 表明是否真的被执行还是遭遇了异常, bool 可以显式暴露非法调用
		bool Begin(SceneMode mode);
		bool Update(Timestep deltaTime);
		bool End();
	
	};


};
