#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/Scene.h"
#include "Ayin/Scene/Entity.h"


namespace Ayin {

	struct SystemContext;

};

namespace Ayin{

	AYIN_API class ISystem{

	public:

		virtual ~ISystem() = default;

		virtual inline void OnAttach() {};
		virtual inline void OnDetach() {};

		virtual inline void OnBegin(const SystemContext& systemContext) {};

		virtual inline void OnPreUpdate(const SystemContext& systemContext) {};
		virtual inline void OnUpdate(const SystemContext& systemContext) {};
		virtual inline void OnPostUpdate(const SystemContext& systemContext) {};
		virtual inline void OnPresentationUpdate(const SystemContext& systemContext) {};

		virtual inline void OnEnd(const SystemContext& systemContext) {};

	};

	// ------------------------------------------------------------------------------------------------

	//x //! 用一个地址来作为 具体 System 类型的标识
	//x using SystemType = const void*;

	//x template<typename System>
	//x	 requires std::derived_from<System, ISystem>&& std::default_initializable<System>
	//x [[nodiscard]] SystemType GetSystemType() noexcept {
	//x	 static const std::byte SystemTypeToke{};
	//x	 return &SystemTypeToke;
	//x }


	using SystemType = entt::id_type;
	using SystemID = SystemType;

	template<typename System>
		requires std::derived_from<System, ISystem>
	[[nodiscard]] SystemType GetSystemType() noexcept{
		
		return entt::type_hash<System>::value();
		
	};


	template<typename System>
		requires std::derived_from<System, ISystem>
	[[nodiscard]] SystemID GetSystemID() noexcept {

		return entt::type_hash<System>::value();

	};


	//Todo: 单例和其它东西一起继承不行啊，让 ISystem 继承单例

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
