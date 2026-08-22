#include <AyinPch.h>

#include "Ayin/System/EditorSession.h"

namespace Ayin {

	EditorSession::EditorSession(Ref<Scene> scene, const SystemPipeline& systemPipeline)
		: m_Scene{ scene }, m_Pipeline{ systemPipeline }, m_EditorWorld{ scene, systemPipeline } {};

	EditorSession::~EditorSession() {

		StopTemporaryWorld();

	};


	World& EditorSession::GetObservedWorld() {

		return m_TemporaryWorld == nullptr
			? m_EditorWorld
			: *m_TemporaryWorld;

	};

	const World& EditorSession::GetObservedWorld() const {

		return m_TemporaryWorld == nullptr
			? m_EditorWorld
			: *m_TemporaryWorld;

	};

	SceneMode EditorSession::GetObservedMode() const {

		return m_TemporaryWorld == nullptr
			? SceneMode::Editor
			: m_TemporaryMode;

	};


	bool EditorSession::BeginSimulation() {

		return BeginSimulation(m_Scene);

	};

	bool EditorSession::BeginSimulation(const Ref<Scene>& scene) {

		return BeginTemporaryWorld(scene, SceneMode::Simulation);

	};

	bool EditorSession::BeginRuntime() {

		return BeginRuntime(m_Scene);

	};

	bool EditorSession::BeginRuntime(const Ref<Scene>& scene) {

		return BeginTemporaryWorld(scene, SceneMode::Runtime);

	};

	bool EditorSession::BeginTemporaryWorld(const Ref<Scene>& scene, const SceneMode mode) {

		if (scene == nullptr || mode == SceneMode::None || !m_Pipeline.IsValid())
			return false;

		StopTemporaryWorld();
		m_TemporaryWorld = CreateScope<World>(scene, m_Pipeline);
		m_TemporaryMode = mode;

		if (!m_TemporaryWorld->BeginWorldExecutionSession(mode)) {
			m_TemporaryWorld.reset();
			m_TemporaryMode = SceneMode::None;
			return false;
		}

		return true;

	};


	void EditorSession::StopTemporaryWorld() {

		if (m_TemporaryWorld != nullptr && m_TemporaryWorld->SessionReady())
			m_TemporaryWorld->EndWorldExecutionSession();

		m_TemporaryWorld.reset();
		m_TemporaryMode = SceneMode::None;

	};


	bool EditorSession::ApplyPipeline(const SystemPipeline& systemPipeline) {

		if (!systemPipeline.IsValid())
			return false;

		// 候选 System、配置和执行计划必须在任何现有 World 结束前完成。
		SystemSchedule candidate = systemPipeline.CreateSchedule();
		if (!candidate.IsBuilt())
			return false;

		// 预检成功后才结束临时 World；如果候选创建失败，当前 Simulation / Runtime 会话保持不变。
		if (m_TemporaryWorld != nullptr)
			StopTemporaryWorld();

		// World 继续独占 Attach / Begin / End / Detach 的时机控制。
		if (!m_EditorWorld.ApplySchedule(std::move(candidate)))
			return false;

		m_Pipeline = systemPipeline;
		return true;

	};

};