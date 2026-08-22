#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/SceneMode.h"
#include "Ayin/System/SystemPipeline.h"
#include "Ayin/System/World.h"

namespace Ayin {

	// 编辑器级会话拥有持久 Pipeline，并按需创建临时 Simulation / Runtime World。
	// World 只保存运行时 Schedule，不承担结构来源的持久所有权。
	class AYIN_API EditorSession {

	public:

		EditorSession(Ref<Scene> scene, const SystemPipeline& systemPipeline);
		~EditorSession();

		EditorSession(const EditorSession&) = delete;
		EditorSession& operator=(const EditorSession&) = delete;

		World& GetEditorWorld() { return m_EditorWorld; };
		const World& GetEditorWorld() const { return m_EditorWorld; };

		World* GetTemporaryWorld() { return m_TemporaryWorld.get(); };
		const World* GetTemporaryWorld() const { return m_TemporaryWorld.get(); };

		World& GetObservedWorld();
		const World& GetObservedWorld() const;
		SceneMode GetObservedMode() const;

		const SystemPipeline& GetPipeline() const { return m_Pipeline; };

		bool BeginSimulation();
		bool BeginSimulation(const Ref<Scene>& scene);
		bool BeginRuntime();
		bool BeginRuntime(const Ref<Scene>& scene);
		void StopTemporaryWorld();

		// Apply 始终提交到持久 EditorWorld；临时 World 只负责结束当前运行会话。
		bool ApplyPipeline(const SystemPipeline& systemPipeline);

	private:

		bool BeginTemporaryWorld(const Ref<Scene>& scene, SceneMode mode);

	private:

		Ref<Scene> m_Scene;
		SystemPipeline m_Pipeline;
		World m_EditorWorld;
		Scope<World> m_TemporaryWorld;
		SceneMode m_TemporaryMode = SceneMode::None;

	};

};