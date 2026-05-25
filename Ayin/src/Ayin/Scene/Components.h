#pragma once

#include "Ayin/Core/Core.h"
#include "Ayin/core/Log.h"
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity
#include <glm/gtx/euler_angles.hpp>

#include "Ayin/Renderer/Texture.h"
#include "Ayin/Scene/SceneCamera.h"

#include "Ayin/Scene/ScriptableEntity.h"

#include "Ayin/Core/ObjectPool.h"

#include "Ayin/Scene/ComponentRegistry.h"

#include <imgui.h>
#include <entt/entt.hpp>
#include <concepts>
#include <glaze/glaze.hpp>


template <>
struct glz::meta<glm::vec3> {
	using T = glm::vec3;
	static constexpr auto value = glz::object("x", &T::x, "y", &T::y, "z", &T::z);
};

template <>
struct glz::meta<glm::vec4> {
	using T = glm::vec4;
	static constexpr auto value = glz::object("x", &T::x, "y", &T::y, "z", &T::z, "w", &T::w);
};

namespace Ayin {

	// ----------ID组件-----------
	struct IDComponent {

		uint64_t ID{};

		IDComponent() = default;
		IDComponent(uint64_t id) : ID{ id } {};
		IDComponent(const IDComponent&) = default;

		static ::entt::id_type ComponentStorageID() { return ::entt::type_hash<IDComponent>::value(); };

		struct glaze {
			using T = IDComponent;
			static constexpr auto value = glz::object(
				"ID", &T::ID
			);
		};

	};
	AYIN_COMPONENT(IDComponent);


	// ----------名称组件-----------
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

		struct glaze {
			using T = TagComponent;
			static constexpr auto value = glz::object("Name", &T::Name);
		};

	};
	AYIN_COMPONENT(TagComponent);
	AYIN_COMPONENTUI(TagComponent, TagComponent::OnGui);


	// ----------Transform组件-----------
	struct TransformComponent {
		
		glm::vec3 Position	{ 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation	{ 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale		{ 1.0f, 1.0f, 1.0f };


		inline operator glm::mat4() {
		
			glm::mat4 translate{ 1.0f }, rotation{ 1.0f }, scale{ 1.0f };

			translate = glm::translate(translate, Position);
			scale = glm::scale(scale, Scale);
			//toMat4内部的旋转顺序是不可指定的，所以达不到我们的Rx*Ry*Rz
			//x rotation = glm::toMat4(glm::quat{ glm::radians(Rotation)});
			/*
			{
				//四元数合成
				glm::quat qX = glm::angleAxis(Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
				glm::quat qY = glm::angleAxis(Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
				glm::quat qZ = glm::angleAxis(Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
				glm::quat finalRotation = qX * qY * qZ;
				rotation = glm::mat4_cast(finalRotation);
			}
			{
				//矩阵合成
				glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
				glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
				rotation = pitch * yaw * roll;
			} 
			*/
			{
				//glm实验性方法
				rotation = glm::eulerAngleXYZ(glm::radians(Rotation.x), glm::radians(Rotation.y), glm::radians(Rotation.z));
			}
			

			return translate * rotation * scale;

		};

		inline const glm::mat4 GetRotationMatrix() const {

			//toMat4内部的旋转顺序是不可指定的，所以达不到我们的Rx*Ry*Rz
			//x glm::toMat4(glm::quat{ glm::radians(Rotation) })
			/*
			{
				//四元数合成
				glm::quat qX = glm::angleAxis(Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
				glm::quat qY = glm::angleAxis(Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
				glm::quat qZ = glm::angleAxis(Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
				glm::quat finalRotation = qX * qY * qZ;
				rotation = glm::mat4_cast(finalRotation);
			}
			{
				//矩阵合成
				glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
				glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
				rotation = pitch * yaw * roll;
			}
			{
				//glm实验性方法
				rotation = glm::eulerAngleXYZ(Rotation.x, Rotation.y, Rotation.z);
			}
			*/

			return glm::eulerAngleXYZ(glm::radians(Rotation.x), glm::radians(Rotation.y), glm::radians(Rotation.z));

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

		struct glaze {
			using T = TransformComponent;
			static constexpr auto value = glz::object(
				"Position", &T::Position,
				"Rotation", &T::Rotation,
				"Scale", &T::Scale
			);
		};

	};
	AYIN_COMPONENT(TransformComponent);
	AYIN_COMPONENTUI(TransformComponent, TransformComponent::OnGui);


	// ----------图片组件-----------
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

		struct glaze {
			using T = SpriteRendererComponent;
			static constexpr auto value = glz::object("Color", &T::Color);
		};

	};
	AYIN_COMPONENT(SpriteRendererComponent);
	AYIN_COMPONENTUI(SpriteRendererComponent, SpriteRendererComponent::OnGui);


	// ----------相机组件-----------
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

		struct glaze {
			using T = CameraComponent;
			static constexpr auto value = glz::object("Camera", &T::Camera);
		};

	};
	AYIN_COMPONENT(CameraComponent);
	AYIN_COMPONENTUI(CameraComponent, CameraComponent::OnGui);
	

	// ----------脚本组件-----------
	struct NativeScriptComponent {

		ScriptableEntity* ScriptableInstance = nullptr;

		std::string ScriptName;
		glz::raw_json ScriptData;
		//x 一开始是不想这么设计的，而是转换成内部定义的中间结构体，该结构 没有外露
		//x 但那样导致一个问题我在通过json反序列化时，NSC必须bind，否者会影响回收操作，这一点注册器解决了。
		//x 其二时ScriptAbleEntity需要所影响的Enity（目前没有涉及复杂的复数个同组件的情况），这一点我目前通过上下文来解决（但后续可能移除）
		//x 其三也是改变思路的核心原因，生命周期，第一初始化会行为会重置脚本实例，所以反序列化的数据回填需要在初始化之后，这一点还好，只要bind的问题解决了就好。
		//x 但麻烦的时OnCreate这样的过程，它们可能涉及对环境的感知，但是原来的序列化方案，要求反序列化是在构建Entity时进行的，所以可能因为环境尚未构建完毕而导致OnCreate行为异常
		//x 简单来说有一些行为涉及到更加广泛的内容，原先的方案，会因其局限性，对反序列化时机产生限制 ，上下文感知带来麻烦，令上下文耦合严重
		//! 所以采用这个方案，先将序列化数据存储起来，延迟反序列化（分步反序列化），使得反序列化时机可以控制，一来避免环境缺失问题，二来方便后续引入更复杂的序列化效果 ，最后交由sceneSerialize来完成，那个位置的上下文充足，甚至可以不再依靠上下文类


		std::function<void()> InstantiateFunction;		//初始化回调
		std::function<void()> DestroyInstanceFunction;	//移除回调
		
		template<typename ScriptType>
			requires std::derived_from<ScriptType, ScriptableEntity> && std::default_initializable<ScriptType>
		void Bind() {

			static ObjectPool<ScriptType> pool;

			if (ScriptableInstance != nullptr) {
				DestroyInstanceFunction();
			}

			InstantiateFunction = [&]() {
					ScriptableInstance = pool.Allocate(); new(ScriptableInstance) ScriptType(); ScriptName = ScriptableInstance->GetScriptName().value_or("None");
				};
			DestroyInstanceFunction = [&]() { pool.Deallocate(static_cast<ScriptType*>(ScriptableInstance)), ScriptableInstance = nullptr; };

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



		std::string write_ScriptData();
		void read_ScriptData(const std::string& json);

		struct glaze {

			using T = NativeScriptComponent;

			static constexpr auto value = glz::object(
				"ScriptName", &T::ScriptName,
				"ScriptData", glz::custom<&T::read_ScriptData, &T::write_ScriptData>
			);
			//! 序列和反序列化时只是简单的存储导ScriptData中，在场景序列化时由场景序列化器SceneSerializer来处理，它会阶段性处理脚本序列化事宜
			//! 这样行为脚本序列化更加可控

		};

	};
	AYIN_COMPONENT(NativeScriptComponent);
	AYIN_COMPONENTUI(NativeScriptComponent, NativeScriptComponent::OnGui);


}
