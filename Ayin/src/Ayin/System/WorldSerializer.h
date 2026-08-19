#pragma once

#include <Ayin/Core/Core.h>
#include <Ayin/Scene/SceneSerializer.h>

#include <Ayin/System/SystemPipelineSerializer.h>

#include <Ayin/System/World.h>

namespace {

	struct WorldJson {

		SceneJson Scene;

		SystemPipelineJson SystemPipeline;

		struct glaze {
			using T = WorldJson;
			static constexpr auto value = glz::object(
				"Scene", &T::Scene,
				"SystemPipeline", &T::SystemPipeline);
		};


	};

}


namespace Ayin {
	
	class AYIN_API WorldSerializer {


	public:

		WorldSerializer() = default;
		WorldSerializer(World* world);

		~WorldSerializer();

		inline void SetWorld(World* world) { m_World = world; };
		inline const World* GetWorld() const { return m_World; };

		void Serializer(const std::string& filepath);
		void SerializerRuntime(const std::string& filepath);
		std::string SerializerToString();

		void Deserializer(const std::string& filepath);
		void DeserializerRuntime(const std::string& filepath);
		void DeserializerFromString(const std::string& jsonStr);

	private:

		WorldJson BuildWorldJson();
		WorldJson BuildWorldJson(std::string_view jsonStr);

	private:

		World* m_World = nullptr;

	};

}
