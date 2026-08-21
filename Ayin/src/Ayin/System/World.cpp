#include <AyinPch.h>

#include "Ayin/System/World.h"

#include "Ayin/Scene/Scene.h"
#include "Ayin/System/SystemContext.h"

namespace Ayin {
	
	World::World(Ref<Scene> scene, const SystemPipeline& systemPipeline)
		:m_ActiveScene{ scene }, m_SystemSchedule{systemPipeline.CreateSchedule()}
	{};

	World::~World() {

		// 未开始过或已经正常结束的 World 无需清理，也不应产生非法调用警告。
		if (SessionReady())
			EndWorldExecutionSession();

	};

	bool World::BeginWorldExecutionSession(SceneMode mode) {

		if (mode == SceneMode::None || m_ActiveScene == nullptr) {
			AYIN_CORE_WARN("Invalid SceneMode or Scene..");
			return false;
		}

		// 防止多次 Begin
		if (m_CurrentMode != SceneMode::None) {
			AYIN_CORE_WARN("多次调用 Begin, 请使用 TransitionMode 进行切换");
			return false;
		}

		m_CurrentMode = mode;
		SystemContext systemContext{.Scene = *m_ActiveScene, .Mode = m_CurrentMode};
		m_SystemSchedule.Begin(systemContext);

		return true;

	};
	bool World::Update(Timestep deltaTime) {

		return Update(deltaTime, nullptr);

	};


	bool World::Update(Timestep deltaTime, EditorCamera* editorCamera) {

		if (!SessionReady()) {
			AYIN_CORE_WARN("Invalid SceneMode or Scene.");
			return false;
		}

		SystemContext context{
			.Scene{*m_ActiveScene},
			.DeltaTime{deltaTime},
			.Mode{m_CurrentMode},
			.Phase{SystemPhase::None},
			.EditorView{editorCamera}
		};

		m_SystemSchedule.Run(context);

		return true;

	};


	bool World::EndWorldExecutionSession() {

		if (!SessionReady()) {
			AYIN_CORE_WARN("Invalid SceneMode or Scene..");
			return false;
		}

		SystemContext systemContext{ .Scene = *m_ActiveScene, .Mode = m_CurrentMode };
		m_SystemSchedule.End(systemContext);
		m_CurrentMode = SceneMode::None;

		return true;

	};


	void World::ResetSchedule(const SystemPipeline& systemPipeline) {
		
		m_SystemSchedule.Clear();
		m_CurrentMode = SceneMode::None;

		systemPipeline.Build(m_SystemSchedule);


	}; 



	bool World::TransitionMode(SceneMode mode) {

		if (mode == SceneMode::None)
			return false;

		// TransitionMode 只负责运行中的切换；首次启动应显式调用 Begin。
		if (!SessionReady()) {
			AYIN_CORE_WARN("模式切换失败，World 尚未开始运行");
			return false;
		}

		if (m_CurrentMode == mode)
			return true;

		if (!EndWorldExecutionSession()) {
			AYIN_CORE_WARN("模式切换失败，旧模式无法关闭");
			return false;
		}

		if (!BeginWorldExecutionSession(mode)) {
			AYIN_CORE_WARN("模式切换失败，新模式无法开启");
			return false;
		}

		return true;

	};


};
