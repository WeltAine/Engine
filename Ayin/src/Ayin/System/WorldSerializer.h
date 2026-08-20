#pragma once

#include <Ayin/Core/Core.h>
#include <Ayin/Scene/SceneSerializer.h>

#include <Ayin/System/SystemScheduleSerializer.h>

#include <Ayin/System/World.h>

namespace Ayin {

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
		WorldSerializer(Ref<World> world);

		~WorldSerializer() = default;

		inline void SetWorld(Ref<World> world) { m_World = world; };
		inline const Ref<World> GetWorld() const { return m_World; };

		bool Serializer(const std::string_view filepath);
		bool SerializerRuntime(const std::string_view filepath);
		std::string SerializerToString();

		bool Deserializer(const std::string_view filepath);
		bool DeserializerRuntime(const std::string_view filepath);
		bool DeserializerFromString(const std::string_view jsonStr);

	private:

		std::optional<WorldJson> BuildWorldJson();
		std::optional<WorldJson> BuildWorldJson(std::string_view jsonStr);

	private:

		Ref<World> m_World = nullptr;

	};

}
