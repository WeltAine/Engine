#pragma once

#include "Ayin/System/ISystem.h"


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
};
