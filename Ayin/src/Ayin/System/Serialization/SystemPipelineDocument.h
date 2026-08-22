#pragma once

#include "Ayin/Serialization/Json/EngineEnumJson.h"

#include <string>
#include <vector>

namespace Ayin {

	// System Pipeline 的 JSON 数据只保存持久化结构，不查询 Registry，也不保存运行时实例。
	struct SystemPipelineEntryDocument {

		SystemTypeKey Type;

		std::vector<SystemPhase> Phases;
		std::vector<SceneMode> Modes;

		int Order = -1;
		::glz::raw_json Configuration = NullConfiguration;

		static constexpr const char* NullConfiguration = "{}";

		struct glaze {
			using T = SystemPipelineEntryDocument;
			static constexpr auto value = glz::object(
				"Type", &T::Type,
				"Phases", &T::Phases,
				"Modes", &T::Modes,
				"Order", &T::Order,
				"Configuration", &T::Configuration);
		};

	};


	struct SystemPipelineDocument {

		std::vector<SystemPipelineEntryDocument> Systems;

		struct glaze {
			using T = SystemPipelineDocument;
			static constexpr auto value = glz::object(
				"Systems", &T::Systems
			);
		};

	};


};
