#include <AyinPch.h>

#include "Ayin/Scene/SceneSerializer.h"

namespace Ayin {


	SceneSerializer::SceneSerializer(const Ref<Scene>& scene) 
		: m_Scene{scene}
	{};


	void SceneSerializer::Serializer(const std::string& filepath) {
	
		if (m_Scene.get() == nullptr)
			return;

		std::ofstream ofs(filepath);
		glz::basic_ostream_buffer<std::ofstream> buffer(ofs);
		


	};
	void SceneSerializer::SerializerRuntime(const std::string& filepath) {};

	void SceneSerializer::Deserializer(const std::string& filepath) {};
	void SceneSerializer::DeserializerRuntime(const std::string& filepath) {};


}
