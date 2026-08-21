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

		return BeginTemporaryWorld(SceneMode::Simulation);

	};

	bool EditorSession::BeginRuntime() {

		return BeginTemporaryWorld(SceneMode::Runtime);

	};

	bool EditorSession::BeginTemporaryWorld(const SceneMode mode) {

		if (mode == SceneMode::None || !m_Pipeline.IsValid())
			return false;

		StopTemporaryWorld();
		m_TemporaryWorld = CreateScope<World>(m_Scene, m_Pipeline);
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

		// EditorWorld 先完成候选 Schedule 构建；失败时临时 World 仍保持原状态。
		if (!m_EditorWorld.ApplyPipeline(systemPipeline))
			return false;

		// Apply 成功后再结束临时 World，避免无效草稿破坏当前运行会话。
		if (m_TemporaryWorld != nullptr)
			StopTemporaryWorld();

		m_Pipeline = systemPipeline;
		return true;

	};

};