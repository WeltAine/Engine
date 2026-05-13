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

#include <imgui.h>
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

		static void OnGui(Entity& entity) {
			auto& tag = entity.GetComponents<Ayin::TagComponent>();
			char buffer[256] = {};
			strncpy_s(buffer, tag.Name.c_str(), sizeof(buffer) - 1);
			if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
				tag.Name = buffer;
			}
		};

		static ::entt::id_type ComponentStorageID() { return ::entt::type_hash<TagComponent>::value(); };
	};
	AYIN_COMPONENT(TagComponent);
	AYIN_COMPONENTUI(TagComponent, TagComponent::OnGui);


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

		static void OnGui(Entity& entity) {
			auto& transform = entity.GetComponents<Ayin::TransformComponent>();
			
			bool transformDirty = false;

			ImGui::DragFloat3("Position", &transform.Position.x, 0.1f);
			ImGui::DragFloat3("Rotation", &transform.Rotation.x, 0.1f);
			ImGui::DragFloat3("Scale", &transform.Scale.x, 0.1f);
		};

		static ::entt::id_type ComponentStorageID() { return ::entt::type_hash<TransformComponent>::value(); };

	};
	AYIN_COMPONENT(TransformComponent);
	AYIN_COMPONENTUI(TransformComponent, TransformComponent::OnGui);


	struct SpriteRendererComponent {
	
		//ToDo 调整一下
		Ref<Texture2D> Texture2D = nullptr;
		glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};
	
		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent& spriteRendererComponent) = default;
		~SpriteRendererComponent() = default;

		static void OnGui(Entity& entity) {
			auto& sprite = entity.GetComponents<Ayin::SpriteRendererComponent>();
			ImGui::ColorEdit4("Color", &sprite.Color.x);
			// Texture2D 暂时不处理
		};

		static ::entt::id_type ComponentStorageID() { return ::entt::type_hash<SpriteRendererComponent>::value(); };

	};
	AYIN_COMPONENT(SpriteRendererComponent);
	AYIN_COMPONENTUI(SpriteRendererComponent, SpriteRendererComponent::OnGui);


	struct CameraComponent {

		using Requires = entt::type_list<TransformComponent>;

		//ToDo 先使用原有的Camera，之后我们需要替换，其中部分参数和Transform重叠
		SceneCamera Camera;

		CameraComponent() = default;
		CameraComponent(const CameraProp& cameraProp)
			:Camera{ cameraProp } 
		{}
		~CameraComponent() = default;

		static void OnGui(Entity& entity) {
			auto& cameraComponent = entity.GetComponents<Ayin::CameraComponent>();

			ImGui::Spacing();
			ImGui::SeparatorText("Camera Properties");

			CameraProp prop = cameraComponent.Camera.GetCameraProp();
			bool propDirty = false;

			const char* camera_mode_items[] = { "Perspective", "Orthogonal" };
			auto cameraType = cameraComponent.Camera.GetCameraType();
			int current_item_index = (int)cameraType - 1;
			if (ImGui::Combo("Camera Type", &current_item_index, camera_mode_items, IM_ARRAYSIZE(camera_mode_items))) {
				prop.Type = static_cast<Ayin::Camera::CameraType>(current_item_index + 1);
				propDirty = true;
			}

			if (ImGui::DragFloat("FOV", &prop.FOV, 1.0f)) {
				propDirty = true;
			}
			if (ImGui::DragFloat("Height", &prop.Height, 1.0f)) {
				propDirty = true;
			}

			ImGui::Text("AspectRatio: %.2f", prop.AspectRatio);

			if (ImGui::DragFloat("Near Plane", &prop.NearPlaneDistance, 0.01f)) {
				propDirty = true;
			}
			if (ImGui::DragFloat("Far Plane", &prop.FarPlaneDistance, 0.1f)) {
				propDirty = true;
			}
			if (propDirty) {
				cameraComponent.Camera.SetProjection(prop);
			}

		};

		static ::entt::id_type ComponentStorageID() { return ::entt::type_hash<CameraComponent>::value(); };

	};
	AYIN_COMPONENT(CameraComponent);
	AYIN_COMPONENTUI(CameraComponent, CameraComponent::OnGui);
	

	
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

		static void OnGui(Entity& entity) {
			auto& nsc = entity.GetComponents<Ayin::NativeScriptComponent>();
			ImGui::Text("Script: %s", nsc.ScriptableInstance ? "Bound" : "None");

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			nsc.ScriptableInstance->OnGui();

		};

		static ::entt::id_type ComponentStorageID() { return ::entt::type_hash<NativeScriptComponent>::value(); };

	};
	AYIN_COMPONENT(NativeScriptComponent);
	AYIN_COMPONENTUI(NativeScriptComponent, NativeScriptComponent::OnGui);
}
