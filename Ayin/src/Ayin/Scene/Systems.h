#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/Scene.h"
#include "Ayin/Scene/Entity.h"


namespace Ayin {

	struct SystemContext;

};

namespace Ayin::Systems {

	class ISystem{

	public:

		virtual ~ISystem() = default;

		virtual inline void OnAttach() {};
		virtual inline void OnDetach() {};

		virtual inline void OnUpdate(const SystemContext& systemContext) {};

	};


	//! 用一个地址来作为 具体 System 类型的标识
	using SystemType = const void*;

	template<typename System>
		requires std::derived_from<System, ISystem>&& std::default_initializable<System>
	[[nodiscard]] SystemType GetSystemType() noexcept {

		static const std::byte SystemTypeToke{};
		return &SystemTypeToke;

	}



	//Todo: 单例和其它东西一起继承不行啊，让 ISystem 继承单例

	// -----------------------------------------------------------------------------------------------------
	class CameraSystem : public ISystem{

	public:
		void OnUpdate(const SystemContext& systemContext) override;

	};

	// -----------------------------------------------------------------------------------------------------
	class ScriptSystem : public ISystem{

	public:
		void OnUpdate(const SystemContext& systemContext) override;

		static void StopAllScript(Scene& scene);

	};


	// -----------------------------------------------------------------------------------------------------
	class RenderSystem : public ISystem{

	public:
		void OnUpdate(const SystemContext& systemContext) override;

	};

	// ------------------------------------------------------------------------------------------------------

	class DestroySystem : public ISystem {

	};
};
