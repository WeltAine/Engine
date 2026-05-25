#pragma once

#include <glaze/glaze.hpp>

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/Scene.h"
#include "Ayin/Scene/Entity.h"

#include <optional>

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

		void Deserializer(const std::string& filepath);
		void DeserializerRuntime(const std::string& filepath);

	private:

		Ref<Scene> m_Scene;

	}; 


	class SceneSerializerContext {
		friend class SceneSerializer;

	public:
		static ::std::optional<Ref<Scene>> GetCurrentScene();
		static ::std::optional<Entity> GetCurrentEntity();

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
