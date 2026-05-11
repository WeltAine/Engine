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
		std::function<void(Entity&)> addComponent;
	};

	class AYIN_API ComponentRegistry {
	public:
		// 注册组件
		template<typename T>
		static void Register(const std::string& name);

		// 注册组件渲染回调
		template<typename T>
		static void RegisterUI(std::function<void(Entity&)> handler);

		// 注册组件添加回调
		template<typename T>
		static void RegisterAddComponent(std::function<void(Entity&)> handler);

		// 获取信息
		template<typename ComponentType>
		static ComponentDescriptor* GetComponentDescriptor();

		static ComponentDescriptor* GetComponentDescriptor(entt::id_type id);

		static void DrawEntityComponents(Entity entity);

		// 可添加的组件
		static std::vector<ComponentDescriptor> GetAvailableComponents(Entity entity);

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
    inline static ::Ayin::detail::ComponentRegistrar<Type> \
        AYIN_CONCAT(_reg_, Type)(#Type)


	//! 关于inline， static，extern
	//! 符号被定义时就意味着产生一个可被访问的数据地址，这些地址之后会被用于编译时链接和访问
	//! 而以上三个符号会决定链接规则
	//				声明 / 定义 ?		链接可见性 ?							ODR （唯一定义原则）重复 ?
	//	─────────────────────────────────────────────────
	//	static        声明(或定义)		内部（类或翻译单元内部）链接			TU 独立
	//	extern        声明(或定义)		外部链接								禁止重复
	//	inline        定义				外部链接								允许重复(合并定义)
	// 
	//	- static — 每个翻译单元都有自己独立的一份，互不干扰。这也是你类内静态成员受限制的根本原因——它必须有一个"唯一的家"
	//	- extern — "这个东西在别处定义了"，跨翻译单元共享同一个符号
	//	- inline — 和 extern 一样外部链接，但允许同一个定义出现在多个翻译单元，链接器负责合并

	//x 这就是为什么这个宏里会有inline，就是为了保证Components.h中的定义被合并
	//x 类内部的一般成变可以内部初始化，是因为每个类实体都有这么些成变地址，但类内static是所有类共享，一旦定义就会在类实体内部产生地址，所以加上inline（C++17）
	//x const static之所以可以直接写在类内部，是因为被立即数优化了
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
			[](Entity&, const std::string&) {},
			[](Entity& entity) { entity.AddComponent<ComponentType>(); },
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
	void ComponentRegistry::RegisterAddComponent(std::function<void(Entity&)> handler) {

		entt::id_type id = entt::type_hash<ComponentType>::value();
		auto& allComponentDescriptors = GetAllComponentDescriptors();

		for (auto& desc : allComponentDescriptors) {
			if (desc.id == id) {
				desc.addComponent = handler;
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




