#include <AyinPch.h>

#include "Ayin/System/WorldSerializer.h"

namespace Ayin {
	

	WorldSerializer::WorldSerializer(Ref<World> world) 
		: m_World{ world } {};


	bool WorldSerializer::Serializer(const std::string_view filepath) {
	
		std::string jsonStr = SerializerToString();


		std::ofstream ofs{std::string{filepath}};//! string_view.data() 只返回指针，没有确切大小信息（尽管 string_view 本体有），所以可能越界读取（不是超过读取范围，而是超过 view 限制的范围）
		if (!ofs.is_open()) {
			AYIN_CORE_ERROR("Failed to open file for writing: {}", filepath);
			return false;
		}
		ofs << jsonStr;

		return true;

	};
	bool WorldSerializer::SerializerRuntime(const std::string_view filepath) {
	
		return Serializer(filepath);

	};
	std::string WorldSerializer::SerializerToString() {
		
		// 构建结构
		std::optional<WorldJson> worldJson = BuildWorldJson();

		if (!worldJson)
			return {};

		auto result = glz::write_json(*worldJson);
		if (!result) {
			AYIN_CORE_ERROR("Failed to serialize World to JSON: {}", glz::format_error(result.error()));

			return {};
		}

		return *result;

	};



	bool WorldSerializer::Deserializer(const std::string_view filepath) {
	
		// 获取反序列化文件数据
		std::ifstream ifs{std::string{filepath}};
		if (!ifs.is_open()) {
			AYIN_CORE_ERROR("Failed to open .world file: {}", filepath);
			return false;
		}

		std::stringstream buffer;
		buffer << ifs.rdbuf();
		const std::string jsonStr = buffer.str();//! buffer.str() 返回的是临时 string 对象，这会导致std::string_view 会悬空

		return DeserializerFromString(jsonStr);

	};
	bool WorldSerializer::DeserializerRuntime(const std::string_view filepath) {
		
		return Deserializer(filepath);

	};
	bool WorldSerializer::DeserializerFromString(const std::string_view jsonStr) {
		
		std::optional<WorldJson> worldJson{BuildWorldJson(jsonStr)};

		if (!worldJson)
			return false;

		// 场景反序列化
		m_World->m_ActiveScene = nullptr;

		SceneSerializer sceneSerializer{ m_World->m_ActiveScene };
		sceneSerializer.DeserializerFrom(worldJson->Scene);

		
		// 系统管线反序列化
		SystemPipeline::Builder builder = SystemScheduleSerializer::Deserializer(worldJson->SystemPipeline);

		m_World->ResetSchedule(builder.Build());

		return true;

	};


	std::optional<WorldJson> WorldSerializer::BuildWorldJson() {
	
		SceneSerializer sceneSerializer{m_World->m_ActiveScene};
		std::optional<SceneJson> sceneJson = sceneSerializer.BuildSceneJson();

		std::optional<SystemPipelineJson> systemPipelineJson = SystemScheduleSerializer::BuildSystemPipelineJson(m_World->m_SystemSchedule);

		WorldJson worldJson;

		if (sceneJson)
			worldJson.Scene = *sceneJson;

		if (systemPipelineJson)
			worldJson.SystemPipeline = *systemPipelineJson;
		
		return worldJson;

	};
	std::optional<WorldJson> WorldSerializer::BuildWorldJson(std::string_view jsonStr) {
			
		WorldJson worldJson;

		auto err = ::glz::read_json(worldJson, jsonStr);
		if (err) {
			AYIN_CORE_ERROR("Failed to parse World JSON: {}", glz::format_error(err, jsonStr));
			return std::nullopt;
		}

		return worldJson;

	};


};
