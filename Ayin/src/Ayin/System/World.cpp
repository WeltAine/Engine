#include <AyinPch.h>

#include "Ayin/System/World.h"

#include "Ayin/Scene/Scene.h"
#include "Ayin/System/SystemContext.h"

namespace Ayin {
	
	World::World(Ref<Scene> scene, const SystemPipeline& systemPipeline)
		:m_ActiveScene{ scene }, m_SystemSchedule{systemPipeline.CreateSchedule()}
	{
		if (m_SystemSchedule.IsBuilt() && !m_SystemSchedule.AttachSystems())
			AYIN_CORE_ERROR("World failed to attach its initial Schedule");
	};

	World::~World() {

		// World 先结束运行会话，再负责 Schedule 的 Detach；Schedule 析构只处理已经 Idle 的对象。
		if (SessionReady() && !EndWorldExecutionSession())
			AYIN_CORE_ERROR("World failed to end its execution session during destruction");

		if (m_SystemSchedule.IsAttached())
			m_SystemSchedule.DetachSystems();

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

		SystemContext systemContext{.Scene = *m_ActiveScene, .Mode = mode};
		if (!m_SystemSchedule.Begin(systemContext))
			return false;

		m_CurrentMode = mode;
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

		if (m_CurrentMode == SceneMode::None)
			return true;

		if (m_ActiveScene == nullptr) {
			AYIN_CORE_WARN("World has no active Scene");
			return false;
		}

		SystemContext systemContext{ .Scene = *m_ActiveScene, .Mode = m_CurrentMode };
		m_SystemSchedule.End(systemContext);
		m_CurrentMode = SceneMode::None;

		return true;

	};


	ISystem* World::FindSystemInstance(const SystemID systemId) {

		return m_SystemSchedule.FindSystemInstance(systemId);

	};


	const ISystem* World::FindSystemInstance(const SystemID systemId) const {

		return m_SystemSchedule.FindSystemInstance(systemId);

	};


	ISystem* World::FindSystemInstance(const std::string_view systemName) {

		return m_SystemSchedule.FindSystemInstance(systemName);

	};


	const ISystem* World::FindSystemInstance(const std::string_view systemName) const {

		return m_SystemSchedule.FindSystemInstance(systemName);

	};


	bool World::ApplyPipeline(const SystemPipeline& systemPipeline) {

		// 候选 Schedule 必须在旧 Schedule 停止前完整构建。
		if (!systemPipeline.IsValid()) {
			AYIN_CORE_ERROR("World Apply failed because the Pipeline is invalid");
			return false;
		}

		SystemSchedule candidate = systemPipeline.CreateSchedule();
		return ApplySchedule(std::move(candidate));

	};


	bool World::ApplySchedule(SystemSchedule&& candidate) {

		if (!candidate.IsBuilt()) {
			AYIN_CORE_ERROR("World Apply failed because the candidate Schedule could not be built");
			return false;
		}

		const bool wasActive = m_SystemSchedule.IsActive();
		const SceneMode previousMode = m_CurrentMode;

		if (wasActive && !EndWorldExecutionSession()) {
			AYIN_CORE_ERROR("World Apply failed because the current execution session could not end");
			return false;
		}

		// Move assignment 不再替调用者 Detach；World 在替换前显式完成旧 Schedule 的清理。
		m_SystemSchedule.DetachSystems();
		SystemSchedule previous = std::move(m_SystemSchedule);
		m_CurrentMode = SceneMode::None;
		m_SystemSchedule = std::move(candidate);

		const auto restorePreviousSchedule = [this, &previous, wasActive, previousMode]() -> bool {
			m_SystemSchedule.DetachSystems();
			m_SystemSchedule = std::move(previous);

			if (!m_SystemSchedule.AttachSystems()) {
				AYIN_CORE_ERROR("World Apply rollback failed because the previous Schedule could not attach");
				m_CurrentMode = SceneMode::None;
				return false;
			}

			if (wasActive && !BeginWorldExecutionSession(previousMode)) {
				AYIN_CORE_ERROR("World Apply rollback failed because the previous Schedule could not begin");
				m_CurrentMode = SceneMode::None;
				return false;
			}

			return true;
		};

		if (!m_SystemSchedule.AttachSystems()) {
			AYIN_CORE_ERROR("World Apply failed because the candidate Schedule could not attach");
			restorePreviousSchedule();
			return false;
		}

		if (wasActive && !BeginWorldExecutionSession(previousMode)) {
			AYIN_CORE_ERROR("World Apply failed because the candidate Schedule could not begin");
			restorePreviousSchedule();
			return false;
		}

		return true;

	};


	void World::ResetSchedule(const SystemPipeline& systemPipeline) {

		ApplyPipeline(systemPipeline);

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