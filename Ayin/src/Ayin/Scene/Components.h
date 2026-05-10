#pragma once

#include "Ayin/Core/Core.h"
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity

#include "Ayin/Renderer/Texture.h"
#include "Ayin/Scene/SceneCamera.h"

#include "Ayin/Scene/ScriptableEntity.h"
#include "Ayin/Core/Timestep.h"

#include "Ayin/Core/ObjectPool.h"

#include "Ayin/Scene/ComponentRegistry.h"

#include <entt/entt.hpp>
#include <concepts>

namespace Ayin {


	struct TagComponent{
	
		std::string Name{"Entity"};

		TagComponent() = default;
		TagComponent(const std::string& name)
			:Name{ name }
		{};
		TagComponent(const TagComponent& tagComponent) = default;

		static ::entt::id_type ComponentPoolID() { return ::entt::type_hash<TagComponent>::value(); };
	};
	AYIN_COMPONENT(TagComponent);


	struct TransformComponent {
		
		glm::vec3 Position	{ 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation	{ 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale		{ 1.0f, 1.0f, 1.0f };


		inline operator glm::mat4() {
		
			glm::mat4 transform{ 1.0f };

			transform = glm::translate(transform, Position);
			transform = glm::scale(transform, Scale);

			glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			return transform = transform * pitch * yaw * roll;

		};

		inline const glm::mat4& GetRotationMatrix() const {
			glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			return pitch * yaw * roll;

		};


		TransformComponent() = default;
		TransformComponent(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
			: Position{ position }, Rotation{ rotation }, Scale{ scale }
		{};
		TransformComponent(const TransformComponent& transformComponent) = default;
		~TransformComponent() = default;

		static ::entt::id_type ComponentPoolID() { return ::entt::type_hash<TransformComponent>::value(); };

	};
	AYIN_COMPONENT(TransformComponent);



	struct SpriteRendererComponent {
	
		//ToDo 调整一下
		Ref<Texture2D> Texture2D = nullptr;
		glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};
	
		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent& spriteRendererComponent) = default;
		~SpriteRendererComponent() = default;

		static ::entt::id_type ComponentPoolID() { return ::entt::type_hash<SpriteRendererComponent>::value(); };

	};
	AYIN_COMPONENT(SpriteRendererComponent);


	struct CameraComponent {

		using Requires = entt::type_list<TransformComponent>;

		//ToDo 先使用原有的Camera，之后我们需要替换，其中部分参数和Transform重叠
		SceneCamera Camera;

		CameraComponent() = default;
		CameraComponent(const CameraProp& cameraProp)
			:Camera{ cameraProp } 
		{}
		~CameraComponent() = default;

		static ::entt::id_type ComponentPoolID() { return ::entt::type_hash<CameraComponent>::value(); };

	};
	AYIN_COMPONENT(CameraComponent);


	
	struct NativeScriptComponent {

		ScriptableEntity* ScriptableInstance = nullptr;


		std::function<void()> InstantiateFunction;
		std::function<void()> DestroyInstanceFunction;
		
		template<typename T>
			requires std::derived_from<T, ScriptableEntity> && std::default_initializable<T>
		void Bind() {

			static ObjectPool<T> pool;

			InstantiateFunction = [&]() { ScriptableInstance = pool.Allocate(); new(ScriptableInstance) T(); };
			DestroyInstanceFunction = [&]() { pool.Deallocate(static_cast<T*>(ScriptableInstance)), ScriptableInstance = nullptr; };

		}

		//ToDo 想一想获取类型T有什么好处，什么事情是必须需要知晓类型的，或者知晓类型会更加轻松
		//x 一个例子（基于多态指针），如果我们想基于prefab创建的话，我们显然不能直接复制指针，但是实际类型我们也不知道
		//x 这就需要用户写一个clone方法了，看上去也不是什么没必要的方法
		//x 但是如果序列化需要脚本类型名称呢？这个方法就很奇怪了
		//x 好吧这些你都能接受，这是修改基类脚本的范式而已，不是什么太糟糕的问题
		//! 那关于运行时的脚本热重载呢？
		//x 这就不是玩笑了，你需要删除旧的脚本指针，然后创建新的，问题就在这里，你不知道之前的实际类型是什么
		//! 所以说白了，只使用多态指针，会丢失实际脚本类型。而我们期望脚本组件和脚本是分离的（这促成了组合基类指针），但是脚本组件的一些运行逻辑需要知晓脚本实际类型



		NativeScriptComponent() = default;
		~NativeScriptComponent() {

			if (ScriptableInstance != nullptr) {

				ScriptableInstance->OnDestroy();
				DestroyInstanceFunction();

			}
			
		}

		static ::entt::id_type ComponentPoolID() { return ::entt::type_hash<NativeScriptComponent>::value(); };

	};
	AYIN_COMPONENT(NativeScriptComponent);
}
