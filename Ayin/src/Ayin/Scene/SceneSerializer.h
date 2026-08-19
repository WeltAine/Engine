#pragma once

#include <glaze/glaze.hpp>

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/Scene.h"
#include "Ayin/Scene/Entity.h"

#include <optional>

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




//! 系统哦是非OOP的，器核心更加聚焦于系统的功能过程，所以关于那些过程中的非自生所长期持有的临时变量，应当由外部提供
// 比如打印机只负责打印，而不是生成纸张
//X （这个比喻不是很好）
// 突然在想为什么SceneSerizlizer的反序列化来返回场景呢？反而是从

namespace Ayin {

	class AYIN_API SceneSerializer {

	public:

		SceneSerializer() = default;
		SceneSerializer(const Ref<Scene>& scene);

		~SceneSerializer();

		inline void SetScene(const Ref<Scene>& scene) { m_Scene = scene; };
		inline const Ref<Scene>& GetScene() const { return m_Scene; };

		void Serializer(const std::string& filepath);
		void SerializerRuntime(const std::string& filepath);
		std::string SerializerToString();

		void Deserializer(const std::string& filepath);
		void DeserializerRuntime(const std::string& filepath);
		void DeserializerFromString(const std::string& jsonStr);

	private:

		std::optional<SceneJson> BuildSceneJson();
		std::optional<SceneJson> BuildSceneJsonFrom(const std::string_view& jsonStr);

	private:

		Ref<Scene> m_Scene;

	}; 


	class SceneSerializerContext {
		friend class SceneSerializer;

	public:
		static ::std::optional<Ref<Scene>> GetCurrentScene();
		static Entity* GetCurrentEntity();

	private:
		static void SetCurrentScene(const Ref<Scene> scene);
		static void EraseSceneContext();

		static void SetCurrentEntity(const Entity& entity);
		static void EraseEntityContext();

	};
	
	struct SceneSerializerContextData {

		Ref<Scene> currentScene;
		Entity* entity;

	};


}
