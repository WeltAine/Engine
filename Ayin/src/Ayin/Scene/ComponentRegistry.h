#pragma once

#include "Ayin/Core/Core.h"
#include "Ayin/Scene/Entity.h"

#include <entt/entt.hpp>
#include <glaze/glaze.hpp>
#include <string>
#include <vector>
#include <functional>

// 用于检测类型是否有本地 Glaze 反射元数据
template<typename T>
concept HasLocalGlazeMeta = requires { T::glaze::value; };

#define AYIN_CONCAT_IMPL(a, b) a##b
#define AYIN_CONCAT(a, b) AYIN_CONCAT_IMPL(a, b)

namespace Ayin {

	struct ComponentDescriptor {
		std::string displayName;
		entt::id_type id;

		std::function<void(Entity&)> onGUI;
		std::function<std::string(Entity&)> serialize;
		std::function<void(Entity&, const std::string&)> deserialize;
		//ToDo 使用反射库，而不是这样别扭的形式添加数据（这里会存在是因为我们没法将id_type转回类型）
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

		static ComponentDescriptor* GetComponentDescriptorByName(const std::string& name);

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

		// 注册组件的UI绘制函数
		template<typename T>
		struct ComponentUIRegistrar {
			ComponentUIRegistrar(std::function<void(Entity&)> onGUI) {
				ComponentRegistry::RegisterUI<T>(onGUI);
			}
		};

	}

	// 注册宏（生成注册结构体）
#define AYIN_COMPONENT(Type) \
    inline static ::Ayin::detail::ComponentRegistrar<Type> AYIN_CONCAT(_reg_, Type)(#Type)

	// 注册UI
#define AYIN_COMPONENTUI(Type, FuncName) \
	inline static ::Ayin::detail::ComponentUIRegistrar<Type> AYIN_CONCAT(_ui_, Type)(FuncName)


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

	// ----- 辅助方法 ------
	template<typename ComponentType>
	std::string SerializeGlazeComponent(Entity& entity) {

		auto result = glz::write_json(entity.GetComponents<ComponentType>());
		if (!result) {
			return "{}";
		}

		return std::move(*result);

	}

	template<typename ComponentType>
	void DeserializeGlazeComponent(Entity& entity, const std::string& json, const std::string& name) {

		if (entity.HasComponents<ComponentType>()) {

			auto err = glz::read_json(entity.GetComponents<ComponentType>(), json);
			if (err) {
				AYIN_CORE_ERROR("Deserialize {} failed: {}", name, glz::format_error(err, json));
			}

		} else {

			auto& comp = entity.AddComponent<ComponentType>();

			auto err = glz::read_json(comp, json);
			if (err) {
				AYIN_CORE_ERROR("Deserialize {} failed: {}", name, glz::format_error(err, json));
			}

		}

	}
	// ---------------------


	template<typename ComponentType>
	void ComponentRegistry::Register(const std::string& name) {

		entt::id_type id = entt::type_hash<ComponentType>::value();

		auto& allComponentDescriptors = GetAllComponentDescriptors();

		for (size_t index = 0; index < allComponentDescriptors.size(); index++) {
			if (allComponentDescriptors[index].id == id) {
				return;
			}
		}

		//注册组件的回调函数
		ComponentDescriptor desc;
		desc.displayName = name;
		desc.id = id;
		desc.onGUI = [](Entity&) {};
		desc.addComponent = [](Entity& entity) { entity.AddComponent<ComponentType>(); };

		if constexpr (HasLocalGlazeMeta<ComponentType>) {

			desc.serialize = &SerializeGlazeComponent<ComponentType>;

			desc.deserialize = [name](Entity& entity, const std::string& json) {
				DeserializeGlazeComponent<ComponentType>(entity, json, name);
			};
			
		} else {
			desc.serialize = [](Entity&) { return "{}"; };
			desc.deserialize = [](Entity&, const std::string&) {};
		}

		allComponentDescriptors.push_back(std::move(desc));

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




