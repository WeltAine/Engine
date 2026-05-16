#include <AyinPch.h>

#include "Ayin/Scene/SceneSerializer.h"
#include "Ayin/Scene/Components.h"
#include "Ayin/Scene/ComponentRegistry.h"

#include <fstream>
#include <sstream>

namespace {

	//! Glaze序列化的是数据内部的阻断，它对组合结构的序列化很自然
	//! 但EnTT的ECS结构，是一种数据结构式的组合方式，并不是类内组合，这使得序列化困难
	//! 我们需要一些顶层结构来真正组合它们，在数据结构式组合和类内组合之间架起桥梁，这就是EntityJsonEntry和SceneJson，这样来让Glaze序列化

	struct EntityJsonEntry {

		uint64_t UUID{};
		std::map<std::string, glz::raw_json> Components;//会导致序列化文件中的组件顺序不固定，但没关系，反序列化时是根据组件名称来找的

		struct glaze {

			using T = EntityJsonEntry;

			static constexpr auto value = glz::object(
				"UUID", &T::UUID,
				"Components", &T::Components
			);

		};

	};

	struct SceneJson {

		std::string SceneName;
		std::vector<EntityJsonEntry> Entities;

		struct glaze {

			using T = SceneJson;

			static constexpr auto value = glz::object(
				"SceneName", &T::SceneName,
				"Entities", &T::Entities
			);

		};

	};

}

namespace Ayin {

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene{scene}
	{};

	void SceneSerializer::Serializer(const std::string& filepath) {

		if (m_Scene.get() == nullptr)
			return;

		SceneJson sceneData;
		sceneData.SceneName = m_Scene->GetName();

		for (auto entityID : m_Scene->m_Registry.storage<entt::entity>()) {

			Entity entity = { entityID, m_Scene.get() };
			if (!entity)
				return;

			EntityJsonEntry entry;
			entry.UUID = entity.GetComponents<IDComponent>().ID;

			for (auto& desc : ComponentRegistry::GetAllComponentDescriptors()) {

				if (entity.HasComponent(desc.id)) {

					std::string compJson = desc.serialize(entity);
					if (!compJson.empty() && compJson != "{}") {
						entry.Components[desc.displayName] = glz::raw_json{ std::move(compJson) };
					}

				}

			}

			sceneData.Entities.push_back(std::move(entry));

		}

		auto result = glz::write_json(sceneData);
		if (!result) {
			AYIN_CORE_ERROR("Failed to serialize scene to JSON: {}", glz::format_error(result.error()));
			return;
		}
		std::string jsonStr = std::move(*result);

		std::ofstream ofs(filepath);
		if (!ofs.is_open()) {
			AYIN_CORE_ERROR("Failed to open file for writing: {}", filepath);
			return;
		}
		ofs << jsonStr;

	};

	void SceneSerializer::SerializerRuntime(const std::string& filepath) {
		//ToDo 目前和编辑时序列化一样，后续可以考虑运行时不序列化一些编辑器专用组件
		Serializer(filepath);
	};

	void SceneSerializer::Deserializer(const std::string& filepath) {

		if (m_Scene.get() == nullptr)
			return;

		std::ifstream ifs(filepath);
		if (!ifs.is_open()) {
			AYIN_CORE_ERROR("Failed to open scene file: {}", filepath);
			return;
		}

		std::stringstream buffer;
		buffer << ifs.rdbuf();
		std::string jsonStr = buffer.str();

		SceneJson sceneData;
		auto err = glz::read_json(sceneData, jsonStr);
		if (err) {
			AYIN_CORE_ERROR("Failed to parse scene JSON: {}", glz::format_error(err, jsonStr));
			return;
		}

		m_Scene->SetName(sceneData.SceneName);

		for (auto& entityEntry : sceneData.Entities) {

			Entity entity = m_Scene->CreateEntity("Entity");

			entity.GetComponents<IDComponent>().ID = entityEntry.UUID;

			for (auto& [compName, rawJson] : entityEntry.Components) {

				auto* desc = ComponentRegistry::GetComponentDescriptorByName(compName);
				if (desc) {
					desc->deserialize(entity, rawJson.str);//反序列化时需要转回字符串
				} else {
					AYIN_CORE_WARN("Unknown component type in scene file: {}", compName);
				}

			}

		}

	};

	void SceneSerializer::DeserializerRuntime(const std::string& filepath) {
		//ToDo 目前和编辑时序列化一样，后续可以考虑运行时不序列化一些编辑器专用组件
		Deserializer(filepath);
	};

}
