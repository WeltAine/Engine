#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/Scene.h"
#include "Ayin/System/SystemSchedule.h"

#include "Ayin/System/SystemPipeline.h"

namespace Ayin {
	
	class World {
	
	private:

		Ref<Scene> m_ActiveScene;
		SystemSchedule m_SystemSchedule;

		SceneMode m_CurrentMode = SceneMode::None;


	public:

		World(Ref<Scene> scene, const SystemPipeline& systemPipeline);
		~World();

		// 返回 bool 表明是否真的被执行还是遭遇了异常, bool 可以显式暴露非法调用
		bool BeginWorldExecutionSession(SceneMode mode);	// 转发到 Schedule 的 Begin 对该模式下所有可以运行的 System 执行 OnBegin
		bool Update(Timestep deltaTime);
		bool EndWorldExecutionSession();					// 转发到 Schedule 的 End 对已经 Begin 的系统进行
	
		inline bool SessionReady() const { return m_CurrentMode != SceneMode::None && m_ActiveScene != nullptr; };

		bool TransitionMode(SceneMode mode);

	};


};
