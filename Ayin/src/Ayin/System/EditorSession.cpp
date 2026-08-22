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

		// Apply 先结束临时 World，确保结构修改统一回到 EditorWorld 提交。
		if (m_TemporaryWorld != nullptr)
			StopTemporaryWorld();

		if (!m_EditorWorld.ApplyPipeline(systemPipeline))
			return false;

		m_Pipeline = systemPipeline;
		return true;

	};

};