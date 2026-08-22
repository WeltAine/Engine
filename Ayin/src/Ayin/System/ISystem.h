#pragma once

#include "Ayin/Core/Core.h"

//! 系统接口头文件

namespace Ayin {

	struct SystemContext;


	AYIN_API class ISystem {

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

		virtual inline void OnEditorGui() {};

	};


};
