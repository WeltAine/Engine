#pragma once

#include"Ayin/Core/Core.h"

#include "Ayin/System/SystemSchedule.h"
#include "Ayin/System/SystemPipeline.h"

namespace {

	struct SystemJson {

		std::string Name;

		std::vector<::Ayin::SystemPhase> Phases;

		std::vector<::Ayin::SceneMode> Modes;

		int Order = -1;											// -1 表示不指定顺序（将由程序以自动递加的顺序设定）

		::glz::raw_json SystemData = NullSystemData;

		static constexpr const char* NullSystemData = "{}";

		struct glaze {
			using T = SystemJson;
			static constexpr auto value = glz::object(
				"Name", &T::Name,
				"Phase", &T::Phases,
				"Modes", &T::Modes,
				"SystemData", &T::SystemData);
		};

	};


	struct SystemPipelineJson {

		std::vector<SystemJson> Systems;

		struct glaze {
			using T = SystemPipelineJson;
			static constexpr auto value = glz::object(
				"Systems", &T::Systems
			);
		};

	};

};


namespace Ayin {

	class AYIN_API SystemPipelineSerializer {

		friend class WorldSetializer;

		SystemPipelineSerializer() = default;
		SystemPipelineSerializer(SystemPipeline* pipeline);

		void SetPipeline(SystemPipeline* pipeline) { m_SystemPipeline = pipeline; };
		SystemPipeline* GetPipeline(SystemPipeline* pipeline) { return m_SystemPipeline; };

	private:

		std::optional<SystemPipelineJson> BuildSystemPipelineJson();
		std::optional<SystemPipelineJson> BuildSystemPipelineJsonFrom(std::string_view jsonStr);
		

	private:

		SystemPipeline* m_SystemPipeline = nullptr;

	};

};
