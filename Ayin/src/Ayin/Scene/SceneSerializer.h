#pragma once

#include <glaze/glaze.hpp>

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/Scene.h"
#include "Ayin/Scene/Entity.h"

namespace Ayin {

	class AYIN_API SceneSerializer {

	public:

		SceneSerializer() = default;
		SceneSerializer(const Ref<Scene>& scene);

		~SceneSerializer() = default;

		inline void SetScene(const Ref<Scene>& scene) { m_Scene = scene; };

		void Serializer(const std::string& filepath);
		void SerializerRuntime(const std::string& filepath);

		void Deserializer(const std::string& filepath);
		void DeserializerRuntime(const std::string& filepath);

	private:

		Ref<Scene> m_Scene;

	};

}
