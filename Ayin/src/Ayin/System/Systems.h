#pragma once

#include "Ayin/System/SystemRegistry.h"


namespace Ayin {

	class Scene;


	// -----------------------------------------------------------------------------------------------------
	class CameraSystem : public ISystem{

	public:
		virtual void OnUpdate(const SystemContext& systemContext) override;

	};

	// -----------------------------------------------------------------------------------------------------
	class ScriptSystem : public ISystem{

	public:

		virtual void OnBegin(const SystemContext& systemContext) override {};

		virtual void OnPreUpdate(const SystemContext& systemContext) override;

		virtual void OnUpdate(const SystemContext& systemContext) override;

		virtual void OnEnd(const SystemContext& systemContext) override;

		static void StopAllScript(Scene& scene);

	};


	// -----------------------------------------------------------------------------------------------------
	class RenderSystem : public ISystem{

	public:

		//ToDo: 相机处理
		virtual void OnBegin(const SystemContext& systemContext) override {};

		virtual void OnUpdate(const SystemContext& systemContext) override;

		virtual void OnEnd(const SystemContext& systemContext) override {};

	};

	// ------------------------------------------------------------------------------------------------------

	class DestroySystem : public ISystem {

	public:
		virtual void OnPreUpdate(const SystemContext& systemContext) override;


	};


	// 内建 System 由本头文件完成静态注册，保证 Editor 和客户端只要包含 Systems.h，
	// 就能够通过稳定 TypeKey 从 SystemRegistry 查询默认 Pipeline 所需的描述符。
	AYIN_SYSTEM(
		DestroySystem,
		"Ayin.System.Destroy",
		"Destroy",
		AYIN_SYSTEMPHASE_LIST(SystemPhase::PreUpdate),
		AYIN_SCENEMODE_LIST(SceneMode::Editor, SceneMode::Simulation, SceneMode::Runtime),
		0
	);

	AYIN_SYSTEM(
		ScriptSystem,
		"Ayin.System.Script",
		"Script",
		AYIN_SYSTEMPHASE_LIST(SystemPhase::PreUpdate, SystemPhase::Update),
		AYIN_SCENEMODE_LIST(SceneMode::Editor, SceneMode::Simulation, SceneMode::Runtime),
		1
	);

	AYIN_SYSTEM(
		CameraSystem,
		"Ayin.System.Camera",
		"Camera",
		AYIN_SYSTEMPHASE_LIST(SystemPhase::Update),
		AYIN_SCENEMODE_LIST(SceneMode::Editor, SceneMode::Simulation, SceneMode::Runtime),
		2
	);

	AYIN_SYSTEM(
		RenderSystem,
		"Ayin.System.Render",
		"Render",
		AYIN_SYSTEMPHASE_LIST(SystemPhase::Update),
		AYIN_SCENEMODE_LIST(SceneMode::Editor, SceneMode::Simulation, SceneMode::Runtime),
		3
	);

};
