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

#include "Ayin/Utils/PlatformUtils.h"

#include "Ayin/Scene/ComponentRegistry.h"

#include <filesystem>
#include <system_error>
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
	//x AYIN_COMPONENT(IDComponent);
	//! 该组件不注册，场景创建实体时一定存在的，这是最基础的，反序列化时是直接变为 Entities 中的 UUID 变量

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



	// ---------- 父子关系组件-----------

	struct RelationShipComponent {

		uint64_t ParentUUID;				// 父节点UUID
		std::vector<uint64_t> ChildrenUUID;	// 子节点UUID


		RelationShipComponent() 
			: ParentUUID{0}
		{};

		RelationShipComponent(uint64_t parentUUID, std::initializer_list<uint64_t> childrenUUID)
			:ParentUUID{ parentUUID }, ChildrenUUID{ childrenUUID }
		{}

		RelationShipComponent(const RelationShipComponent& relationshipComponent) = default; 	//? 这个函数是否该存在存疑，因为两个实体不应该有相同的子对象，不过我们现在目前也没有关于实体复制的功能（随让它很实用，不过之后再说吧）

		static ::entt::id_type ComponentStorageID() { return ::entt::type_hash<RelationShipComponent>::value(); };

		struct glaze {

			using T = RelationShipComponent;

			static constexpr auto value = glz::object(
				"ParentUUID", &T::ParentUUID,
				"ChildrenUUID", &T::ChildrenUUID
			);

		};

	};
	AYIN_COMPONENT(RelationShipComponent);



	// ----------Transform组件-----------
	struct TransformComponent {
		
		glm::vec3 Position	{ 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation	{ 0.0f, 0.0f, 0.0f }; // radians
		glm::vec3 Scale		{ 1.0f, 1.0f, 1.0f };


		inline operator glm::mat4() {
		
			glm::mat4 translate{ 1.0f }, rotation{ 1.0f }, scale{ 1.0f };

			translate = glm::translate(translate, Position);
			scale = glm::scale(scale, Scale);
			{
				//glm实验性方法
				rotation = glm::eulerAngleXYZ(Rotation.x, Rotation.y, Rotation.z);
			}
			

			return translate * rotation * scale;

		};

		inline const glm::mat4 GetRotationMatrix() const {

			return glm::eulerAngleXYZ(Rotation.x, Rotation.y, Rotation.z);

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
			glm::vec3 rotationDegrees = glm::degrees(transform.Rotation);
			if (ImGui::DragFloat3("Rotation (deg)", &rotationDegrees.x, 0.1f)) {
				transform.Rotation = glm::radians(rotationDegrees);
			}
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
		static constexpr const char* NoneTexturePath = "None";

		std::string FilePath = NoneTexturePath;
		Ref<Texture2D> Texture2D = nullptr;
		glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};
	
		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent& spriteRendererComponent) = default;
		~SpriteRendererComponent() = default;

		static void OnGui(Entity& entity) {
			auto& sprite = entity.GetComponents<Ayin::SpriteRendererComponent>();
			ImGui::ColorEdit4("Color", &sprite.Color.x);

			char buffer[512] = {};
			strncpy_s(buffer, sprite.FilePath.c_str(), sizeof(buffer) - 1);
			if (ImGui::InputText("Texture Path", buffer, sizeof(buffer))) {
				sprite.FilePath = buffer;
			}
			if (ImGui::IsItemDeactivatedAfterEdit()) {
				sprite.SetTexture(sprite.FilePath);
			}

			if (ImGui::Button("Browse Texture")) {
				auto filePath = FileDialogs::OpenFile({ {"Texture", "png,jpg,jpeg,bmp,tga"} }, "assets/textures");
				if (filePath) {
					sprite.SetTexture(*filePath);
				}
			}
		};

		static ::entt::id_type ComponentStorageID() { return ::entt::type_hash<SpriteRendererComponent>::value(); };

		inline void SetTexture(const std::string& filePath) {

			// 保持序列化路径和运行时纹理引用同步。
			std::string texturePath = NormalizeTexturePath(filePath);
			if (texturePath.empty() || texturePath == NoneTexturePath) {
				FilePath = NoneTexturePath;
				Texture2D = nullptr;
				return;
			}

			FilePath = texturePath;
			std::error_code error;
			if (!std::filesystem::exists(texturePath, error)) {
				AYIN_CORE_WARN("Texture file does not exist: {0}", texturePath);
				Texture2D = nullptr;
				return;
			}

			Texture2D = Texture2DLibrary::Load(texturePath);

		};

		// 绝对路径相对化
		static std::string NormalizeTexturePath(const std::string& filePath) {

			if (filePath.empty() || filePath == NoneTexturePath)
				return NoneTexturePath;

			std::filesystem::path path{ filePath };	//路径对象，不检查文件是否存在，只是解析路径字符串，如盘符、目录、文件名、分隔符
			std::error_code error;					//文件系统函数运行错误
			if (path.is_absolute()) {				//是否为绝对路径
				std::filesystem::path workingDirectory = std::filesystem::current_path(error);					//当前进程工作目录
				error.clear();	//复用错误，所以清除一下
				std::filesystem::path relativePath = std::filesystem::relative(path, workingDirectory, error);	// path 相对于 workingDirctory 的相对目录，premake 中调整了工作目录
				if (!error && !relativePath.empty() && *relativePath.begin() != "..") {
					//无错误，相对路径非空，相对路径不以..开头（因为我们的工作目录已经调到了项目根目录，如果再有 .. 则说明脱离项目文件范围了）
					path = relativePath;
				}
			}

			return path.generic_string();

		};

		inline void read_FilePath(const std::string& filePath) {
			SetTexture(filePath);
		};

		inline std::string write_FilePath() {
			return FilePath;
		};

		struct glaze {
			using T = SpriteRendererComponent;
			static constexpr auto value = glz::object(
				"FilePath", glz::custom<&T::read_FilePath, &T::write_FilePath>, 
				"Color", &T::Color);
		};

	};
	AYIN_COMPONENT(SpriteRendererComponent);
	AYIN_COMPONENTUI(SpriteRendererComponent, SpriteRendererComponent::OnGui);


	// ----------相机组件-----------
	struct CameraComponent {

		using Requires = entt::type_list<TransformComponent>;

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

			float fovDegrees = glm::degrees(prop.FOVRadians);
			if (ImGui::DragFloat("FOV (deg)", &fovDegrees, 1.0f)) {
				prop.FOVRadians = glm::radians(fovDegrees);
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
	//! 其维护一个不变量，即 Script 的生命周期过程，保证其顺序的合理性，对多调用点的幂等性
	//! 第二个不变量就是 Bind 的类型和实际的 Script 具体类型时刻保持一致
	struct NativeScriptComponent {

		//! 通过状态标记维护 Script 的生命周期流程
		enum class ScriptLifecycleState : uint8_t {

			Unbound,		// 未配置类型或已执行ReleasseInstance
			Bound,			// 已配置类型
			Instantiated,	// 已有实例，但未进入任何生命周期
			Active,			// 已执行 OnCreate
			Stopping

		};


	public:
		static constexpr const char* NoneScriptName = "none";
		static constexpr const char* NullScriptData = "null";

		ScriptableEntity* ScriptableInstance = nullptr;

		std::string ScriptName{ NoneScriptName };//它基本上是实时的
		glz::raw_json ScriptData{ NullScriptData };//只会在序列化和反序列化是被赋值，所以小心赃值。在未bind和非法脚本时未null
		//x 一开始是不想这么设计的，而是转换成内部定义的中间结构体，该结构 没有外露
		//x 但那样导致一个问题我在通过json反序列化时，NSC必须bind，否者会影响回收操作，这一点注册器解决了。
		//x 其二时ScriptAbleEntity需要所影响的Enity（目前没有涉及复杂的复数个同组件的情况），这一点我目前通过上下文来解决（但后续可能移除）
		//x 其三也是改变思路的核心原因，生命周期，第一初始化会行为会重置脚本实例，所以反序列化的数据回填需要在初始化之后，这一点还好，只要bind的问题解决了就好。
		//x 但麻烦的时OnCreate这样的过程，它们可能涉及对环境的感知，但是原来的序列化方案，要求反序列化是在构建Entity时进行的，所以可能因为环境尚未构建完毕而导致OnCreate行为异常
		//x 简单来说有一些行为涉及到更加广泛的内容，原先的方案，会因其局限性，对反序列化时机产生限制 ，上下文感知带来麻烦，令上下文耦合严重
		//! 所以采用这个方案，先将序列化数据存储起来，延迟反序列化（分步反序列化），使得反序列化时机可以控制，一来避免环境缺失问题，二来方便后续引入更复杂的序列化效果 ，最后交由sceneSerialize来完成，那个位置的上下文充足，甚至可以不再依靠上下文类

	private:
		ScriptLifecycleState m_State = ScriptLifecycleState::Unbound;	// 保持私有，方式被意外篡改而影响生命周期

	private:
		std::function<void(NativeScriptComponent& nsc)> InstantiateFunction;		//初始化回调
		std::function<void(ScriptableEntity* scriptInstance)> DestroyInstanceFunction;	//移除回调
		
	public:
		//! 这是 Script 毁灭的开始，尽管它不叫什么 Destory 但是这会改变 Nsc 的记录，导致 Script 以不合理的方式析构 或者 Script 被提前析构（我直接采取了拒绝的方式）
		//! 所以不要在自己的 OnCreate、OnUpdate 和 OnDestory 中对自己的 Nsc 调用 Bind 或者 StopScript 这可能造成超出意料的影响。
		template<typename ScriptType>
			requires std::derived_from<ScriptType, ScriptableEntity> && std::default_initializable<ScriptType>
		inline void Bind() {

			static ObjectPool<ScriptType> pool;

			if (m_State != ScriptLifecycleState::Unbound && m_State != ScriptLifecycleState::Bound)
				return;

			// 更新赋名
			auto scriptName = ScriptType{}.GetScriptName();
			ScriptName = (scriptName && !scriptName->empty()) ? *scriptName : NoneScriptName;

			// 注册新回调
			InstantiateFunction = [](NativeScriptComponent& nsc) {
					nsc.ScriptableInstance = pool.Allocate(); 
					new(nsc.ScriptableInstance) ScriptType(); 
					nsc.ScriptName = nsc.ScriptableInstance->GetScriptName().value_or(NoneScriptName);
				};
			
			DestroyInstanceFunction = [](ScriptableEntity* scriptInstance) { pool.Deallocate(static_cast<ScriptType*>(scriptInstance)); };

			m_State = ScriptLifecycleState::Bound;

		}

		inline ScriptableEntity* Instantiate() {

			if ( m_State != ScriptLifecycleState::Bound ) {
				AYIN_CORE_ASSERT(false, "Script is not bound");
				return nullptr;
			}

			if (!ScriptableInstance && InstantiateFunction) {
				this->InstantiateFunction(*this);

				if(ScriptableInstance)
					m_State = ScriptLifecycleState::Instantiated;
			}

			return this->ScriptableInstance;
		
		}

		inline void ActiveScript(const Entity& entity = Entity{}) {
		
			if (m_State != ScriptLifecycleState::Instantiated) {
				AYIN_CORE_ASSERT(false, "Script LifeLoop Error: ScriptableInstance is null");
				return;
			}

			ScriptableInstance->SetEntity(entity);
			ScriptableInstance->OnCreate();

			m_State = ScriptLifecycleState::Active;

		};

		inline void Update(Timestep deltaTime) { 

			if (m_State != ScriptLifecycleState::Active) {
				AYIN_CORE_ASSERT(false, "Script LifeLoop Error");
				return;
			}

			ScriptableInstance->OnUpdate(deltaTime); 
		
		};

		inline void StopScript() {

			if (m_State != ScriptLifecycleState::Active && m_State != ScriptLifecycleState::Instantiated) {
				return;
			}

			if (m_State == ScriptLifecycleState::Active) {
				m_State = ScriptLifecycleState::Stopping;
				ScriptableInstance->OnDestroy();
			}

			ReleaseInstance();

		};


		// 是否绑定了脚本
		inline bool HasScript() const {
			return !ScriptName.empty() && ScriptName != NoneScriptName;
		}

		inline ScriptLifecycleState GetScriptLifecycleState() const { return m_State; };


		NativeScriptComponent() = default;
		NativeScriptComponent(NativeScriptComponent&& other) noexcept
			: ScriptableInstance{
				  std::exchange(other.ScriptableInstance, nullptr)
			},
			m_State{other.m_State},
			ScriptName{ std::move(other.ScriptName) },
			ScriptData{ std::move(other.ScriptData) },
			InstantiateFunction{ std::move(other.InstantiateFunction) },
			DestroyInstanceFunction{ std::move(other.DestroyInstanceFunction) }
		{}

		NativeScriptComponent(const NativeScriptComponent&) = delete;
		NativeScriptComponent& operator=(const NativeScriptComponent&) = delete;
		NativeScriptComponent& operator=(NativeScriptComponent&&) = delete;

		inline ~NativeScriptComponent() {

			if (ScriptableInstance != nullptr) {
				StopScript();
			}
			
		}

		inline static void OnGui(Entity& entity) {
			auto& nsc = entity.GetComponents<Ayin::NativeScriptComponent>();
			ImGui::Text("Script: %s", nsc.HasScript() ? nsc.ScriptName.c_str() : "None");

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (nsc.ScriptableInstance != nullptr) {
				nsc.ScriptableInstance->OnGui();
			}

		};

		inline static ::entt::id_type ComponentStorageID() { return ::entt::type_hash<NativeScriptComponent>::value(); };


	private:
		inline void ReleaseInstance() {
			
			if (m_State != ScriptLifecycleState::Stopping && m_State != ScriptLifecycleState::Instantiated) {
				AYIN_CORE_ASSERT(false, "Script LifeLoop Error");
				return;
			}


			AYIN_CORE_ASSERT(this->DestroyInstanceFunction, "Script '{}' is not bound", this->ScriptName);

			ScriptableEntity* instance = std::exchange(ScriptableInstance, nullptr);
			//! std::exchange ：取出变量的旧值，同时用新值替换它。

			if (instance && DestroyInstanceFunction)
				DestroyInstanceFunction(instance);

			m_State = ScriptLifecycleState::Bound;
		}

	public:

		glz::raw_json write_ScriptData();
		void read_ScriptData(glz::raw_json json);

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
