#pragma once

#include "Ayin/Core/Core.h"
#include "Ayin/Scene/Entity.h"

#include <entt/entt.hpp>
#include <string>
#include <vector>
#include <functional>

#define AYIN_CONCAT_IMPL(a, b) a##b
#define AYIN_CONCAT(a, b) AYIN_CONCAT_IMPL(a, b)

namespace Ayin {

	class Entity;

	struct ComponentDescriptor {
		std::string displayName;
		entt::id_type id;

		//const修饰后能使用回调么？
		std::function<void(Entity&)> onGUI;
		std::function<std::string(const Entity&)> serialize;
		std::function<void(const Entity&, const std::string&)> deserialize;
	};

	class AYIN_API ComponentRegistry {
	public:
		// 注册组件
		template<typename T>
		static void Register(const std::string& name);

		// 注册组件渲染回调
		template<typename T>
		static void RegisterUI(std::function<void(Entity&)> handler);

		// 获取信息
		template<typename ComponentType>
		static ComponentDescriptor* GetComponentDescriptor();

		static void DrawEntityComponents(Entity entity);

		// 可添加的组件
		static std::vector<ComponentDescriptor> GetAvailableComponents(Entity entity);

	private:
		static std::vector<ComponentDescriptor>& GetAllComponentDescriptors();
	};

	namespace detail {
		// 注册结构体（用该结构体的默认构造包装注册过程）
		template<typename T>
		struct ComponentRegistrar {
			ComponentRegistrar(const char* name) {
				ComponentRegistry::Register<T>(name);
			}
		};
	}

	// 注册宏（生成注册结构体）
#define AYIN_COMPONENT(Type) \
    static ::Ayin::detail::ComponentRegistrar<Type> \
        AYIN_CONCAT(_reg_, __LINE__)(#Type)


}


namespace Ayin {

	template<typename ComponentType>
	void ComponentRegistry::Register(const std::string& name) {

		entt::id_type id = entt::type_hash<ComponentType>::value();

		auto& allComponentDescriptors = GetAllComponentDescriptors();

		for (size_t index = 0; index < allComponentDescriptors.size(); index++) {
			if (allComponentDescriptors[index].id == id) {
				return;
			}
		}

		allComponentDescriptors.push_back({
			name,
			id,
			[](Entity&) {},
			[](const Entity&) { return "{}"; },
			[](const Entity&, const std::string&) {}
			});
	}

	template<typename ComponentType>
	void ComponentRegistry::RegisterUI(std::function<void(Entity&)> handler) {

		entt::id_type id = entt::type_hash<ComponentType>::value();

		auto& allComponentDescriptors = GetAllComponentDescriptors();

		for (auto& componentDescriptor : allComponentDescriptors) {
			if (componentDescriptor.id == id) {
				componentDescriptor.onGUI = handler;
				return;
			}
		}

	}

	template<typename ComponentType>
	ComponentDescriptor* ComponentRegistry::GetComponentDescriptor() {

		entt::id_type id = entt::type_hash<ComponentType>::value();

		auto& allComponentDescriptors = GetAllComponentDescriptors();

		for (size_t index = 0; index < allComponentDescriptors.size(); index++) {

			if (allComponentDescriptors[index].id == id) {
				return &allComponentDescriptors[index];
			}

		}

		AYIN_CORE_ASSERT(false, "This Component didn't register!");
		return nullptr;

	};


}




