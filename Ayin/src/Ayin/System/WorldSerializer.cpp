#include <AyinPch.h>

#include "Ayin/System/WorldSerializer.h"

namespace Ayin {
	

	WorldSerializer::WorldSerializer(Ref<World> world) 
		: m_World{ world } {};


	bool WorldSerializer::Serializer(const std::string_view filepath) {
	
		std::string jsonStr = SerializerToString();


		std::ofstream ofs{filepath.data()};
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
		std::ifstream ifs{ filepath.data()};
		if (!ifs.is_open()) {
			AYIN_CORE_ERROR("Failed to open .world file: {}", filepath);
			return;
		}

		std::stringstream buffer;
		buffer << ifs.rdbuf();
		std::string_view jsonStr = buffer.str();

		DeserializerFromString(jsonStr);

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
