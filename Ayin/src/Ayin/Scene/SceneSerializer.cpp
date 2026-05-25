#include <AyinPch.h>

#include "Ayin/Scene/SceneSerializer.h"
#include "Ayin/Scene/Components.h"
#include "Ayin/Scene/ComponentRegistry.h"
#include "Ayin/Scene/ScriptRegistry.h"

#include <fstream>
#include <sstream>
#include <map>


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
	{
		SceneSerializerContext::SetCurrentScene(m_Scene);
	};

	SceneSerializer::~SceneSerializer() {
		SceneSerializerContext::EraseSceneContext();
	}

	void SceneSerializer::Serializer(const std::string& filepath) {

		if (m_Scene.get() == nullptr)
			return;

		// 创建场景序列化中间结构
		SceneJson sceneData;
		sceneData.SceneName = m_Scene->GetName();

		// 逐个处理场景中Entity
		for (auto entityID : m_Scene->m_Registry.storage<entt::entity>()) {

			Entity entity = { entityID, m_Scene.get() };
			if (!entity)
				return;

			SceneSerializerContext::SetCurrentEntity(entity);

			// 创建Entity序列化中间结构
			EntityJsonEntry entry;
			entry.UUID = entity.GetComponents<IDComponent>().ID;

			// 处理Enity拥有的所有组件
			for (auto& desc : ComponentRegistry::GetAllComponentDescriptors()) {
				// 获取组件行为

				if (entity.HasComponent(desc.id)) {

					std::string compJson = desc.serialize(entity);
					if (!compJson.empty() && compJson != "{}") {
						entry.Components[desc.displayName] = glz::raw_json{ std::move(compJson) };
					}

				}

			}

			sceneData.Entities.push_back(std::move(entry));

		}

		// 序列化中间结构
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

		SceneSerializerContext::EraseEntityContext();

	};

	void SceneSerializer::SerializerRuntime(const std::string& filepath) {
		//ToDo 目前和编辑时序列化一样，后续可以考虑运行时不序列化一些编辑器专用组件
		Serializer(filepath);
	};

	void SceneSerializer::Deserializer(const std::string& filepath) {

		// 确保有可以反序列化的对象
		if (m_Scene.get() == nullptr)
			return;

		// 获取反序列化文件数据
		std::ifstream ifs(filepath);
		if (!ifs.is_open()) {
			AYIN_CORE_ERROR("Failed to open scene file: {}", filepath);
			return;
		}

		std::stringstream buffer;
		buffer << ifs.rdbuf();
		std::string jsonStr = buffer.str();

		// 构建反序列化中间层
		SceneJson sceneData;
		auto err = glz::read_json(sceneData, jsonStr);
		if (err) {
			AYIN_CORE_ERROR("Failed to parse scene JSON: {}", glz::format_error(err, jsonStr));
			return;
		}

		// 反序列化回场景
		m_Scene->SetName(sceneData.SceneName);

		for (auto& entityEntry : sceneData.Entities) {

			Entity entity = m_Scene->CreateEntity("Entity");

			SceneSerializerContext::SetCurrentEntity(entity);

			entity.GetComponents<IDComponent>().ID = entityEntry.UUID;

			for (auto& [compName, rawJson] : entityEntry.Components) {

				auto* desc = ComponentRegistry::GetComponentDescriptorByName(compName);// 获取组件行为
				if (desc) {
					desc->deserialize(entity, rawJson.str);//反序列化时需要转回字符串
				} else {
					AYIN_CORE_WARN("Unknown component type in scene file: {}", compName);
				}

			}

		}

		SceneSerializerContext::EraseEntityContext();

		auto&& nativeScriptComponentView = m_Scene->m_Registry.view<NativeScriptComponent>();

		// 初始化脚本实例
		nativeScriptComponentView.each([=](entt::entity entity, NativeScriptComponent& nsc) {
				if (!nsc.HasScript()) {
					return;
				}

				nsc.InstantiateFunction();
				if (nsc.ScriptableInstance != nullptr) {
					nsc.ScriptableInstance->m_Entity = Entity{ entity, m_Scene.get() };
				}
			});

		// 脚本实例反序列
		nativeScriptComponentView.each([=](entt::entity, NativeScriptComponent& nsc) {
				if (nsc.ScriptableInstance == nullptr) {
					return;
				}

				ScriptRegistry::DeserializeScriptByScriptName(nsc, nsc.ScriptName, nsc.ScriptData.str);
			});

		// OnCreate生命周期
		nativeScriptComponentView.each([=](entt::entity, NativeScriptComponent& nsc) {
				if (nsc.ScriptableInstance != nullptr) {
					nsc.ScriptableInstance->OnCreate();
				}
			});


	};

	void SceneSerializer::DeserializerRuntime(const std::string& filepath) {
		//ToDo 目前和编辑时序列化一样，后续可以考虑运行时不序列化一些编辑器专用组件
		Deserializer(filepath);
	};



	static SceneSerializerContextData s_SceneSerializerContextData;

	void SceneSerializerContext::SetCurrentScene(const Ref<Scene> scene) { s_SceneSerializerContextData.currentScene = scene; };
	::std::optional<Ref<Scene>> SceneSerializerContext::GetCurrentScene() { return s_SceneSerializerContextData.currentScene; };
	void SceneSerializerContext::EraseSceneContext() { s_SceneSerializerContextData.currentScene = nullptr; };

	void SceneSerializerContext::SetCurrentEntity(const Entity& entity) { s_SceneSerializerContextData.entity = &const_cast<Entity&>(entity); };
	Entity* SceneSerializerContext::GetCurrentEntity() { return s_SceneSerializerContextData.entity; };
	void SceneSerializerContext::EraseEntityContext() { s_SceneSerializerContextData.entity = nullptr; };


}
