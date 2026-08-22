#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/System/Serialization/SystemPipelineDocument.h"
#include "Ayin/System/SystemPipeline.h"

#include <optional>
#include <string_view>

namespace Ayin {

	class SystemSchedule;

	// Schedule 与 PipelineDocument 之间的数据映射。
	// 不负责文件 I/O；从 Document 读取时只生成可继续编辑的 Builder。
	class AYIN_API SystemScheduleSerializer {

	public:

		static std::optional<SystemPipeline::Builder> Deserialize(const SystemPipelineDocument& document);

		static std::optional<SystemPipelineDocument> Serialize(const SystemSchedule& schedule);
		static std::optional<SystemPipelineDocument> Parse(std::string_view jsonStr);

	};


};
