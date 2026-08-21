#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/System/SystemTypes.h"

#include <glaze/glaze.hpp>

#include <concepts>
#include <functional>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

namespace Ayin {

	struct SystemRegistration;

	// 系统是否具备独立的序列化要求
	template<typename System>
	concept HasSystemGlazeMeta = requires{System::glaze::value; };

	
	struct AYIN_API SystemDescriptor {

		static constexpr const char* NullSystemData = "{}";

		SystemInformation Information;
		SystemSpecification DefaultSpecification;

		std::function<Scope<ISystem>()> CreateSystem;
		std::function<std::string(const Scope<ISystem>&)> SerializeSystem;					// 序列化 系统的 自有数据
		std::function<bool(Scope<ISystem>&, const std::string&)> DeserializeSystem;			// 反序列化 系统改的自有数据

		// 支持从 描述符 转换为对应 注册配置
		operator SystemRegistration() const;

		};



	class AYIN_API SystemRegistry {

	public:

		template<typename System>
			requires std::derived_from<System, ISystem>&& std::default_initializable<System>
		static void Registry(const std::initializer_list<SystemPhase>& defaultPhases, const std::initializer_list<SceneMode>& defaultModes, int order);

		static Scope<ISystem> CreateSystemBy(const std::string_view systemName);
		static Scope<ISystem> CreateSystemBy(SystemID systemId);
		static std::string SerializeSystem(const Scope<ISystem>& system, std::string_view systemName);
		static std::string SerializeSystem(const Scope<ISystem>& system, const SystemID systemId);
		static bool DeserializeSystem(Scope<ISystem>& system, std::string_view systemName, const std::string& json);
		static bool DeserializeSystem(Scope<ISystem>& system, const SystemID systemId, const std::string& json);


		static const std::vector<SystemDescriptor>& GetAllSystemDescriptors();
		static const SystemDescriptor* GetSystemDescriptor(std::string_view systemName);
		static const SystemDescriptor* GetSystemDescriptor(SystemID systemId);

	private:

		static std::vector<SystemDescriptor>& GetAllSystemDescriptorsMutable();

	};


	template<typename System>
		requires std::derived_from<System, ISystem>&& std::default_initializable<System>
	void SystemRegistry::Registry(const std::initializer_list<SystemPhase>& defaultPhases, const std::initializer_list<SceneMode>& defaultModes, int order) {

		// 系统信息
		SystemInformation information{
			.RuntimeId{GetSystemID<System>()},
			.Name{typeid(System).name()}
		};

		// 系统参数
		SystemPhase phaseMask = SystemPhase::None;
		for (const auto& phase : defaultPhases) {
			phaseMask |= phase;
		}

		SceneMode modeMask = SceneMode::None;
		for (const auto& mode : defaultModes) {
			modeMask |= mode;
		}

		SystemSpecification specification{
			.PhaseMask{phaseMask},
			.ModeMask{modeMask},
			.Order{order}
		};


		// 系统回调
		std::function<Scope<ISystem>()> createSystem =
			[]() -> Scope<ISystem> {

			return CreateScope<System>();//！ std::unique_ptr 提供的 converting move constructor，即转换移动构造。（当模板类型具备继承关系时可以转换）

			};

		std::function<std::string(const Scope<ISystem>&)> serializeSystem =
			[](const Scope<ISystem>& system_ptr) ->std::string {

			if constexpr (HasSystemGlazeMeta<System>) {

				if (system_ptr) {

					const System* system = static_cast<const System*>(system_ptr.get());//? const Scopr<ISystem> 的 get 返回的就是 const System* 么

					auto result = ::glz::write_json(*system);
					if (!result) {
						return SystemDescriptor::NullSystemData;
					}

					return *result;

				}
				else {
					return SystemDescriptor::NullSystemData;
				}

			}

			return SystemDescriptor::NullSystemData;

			};

		std::function<bool(Scope<ISystem>&, const std::string&)> deserializeSystem =
			[](Scope<ISystem>& system_ptr, const std::string& json) -> bool {

			if constexpr (HasSystemGlazeMeta<System>) {

				if (system_ptr.get()) {

					System* system = static_cast<System*>(system_ptr.get());

					auto err = ::glz::read_json(*system, json);
					if (err) {
						AYIN_CORE_ERROR("Deserialize failed: {}", glz::format_error(err, json));
						return false;
					}

					return true;

				}
				else {
					return false;
				}

			}

			return json.empty() || json == SystemDescriptor::NullSystemData;

			};


		// 系统描述符封装
		GetAllSystemDescriptorsMutable().emplace_back(
			SystemDescriptor{
				.Information{std::move(information)},
				.DefaultSpecification{std::move(specification)},
				.CreateSystem{std::move(createSystem)},
				.SerializeSystem{std::move(serializeSystem)},
				.DeserializeSystem{std::move(deserializeSystem)}
			}
		);

	};



	namespace detail {
	

		template<typename System>
			requires std::derived_from<System, ISystem>&& std::default_initializable<System>
		struct AYIN_API SystemRegistrar {

			inline SystemRegistrar(const std::initializer_list<SystemPhase>& defaultPhases, const std::initializer_list<SceneMode>& defaultModes, int order) {
				::Ayin::SystemRegistry::Registry<System>(defaultPhases, defaultModes, order);
			}

		};
	}


#define AYIN_SYSTEMPHASE_LIST(...) \
	{ __VA_ARGS__ }

#define AYIN_SCENEMODE_LIST(...) \
	{ __VA_ARGS__ }

#define AYIN_SYSTEM(System, Phases_List, Modes_list, Order) \
		inline static ::Ayin::detail::SystemRegistrar<System> AYIN_CONCAT(_reg_, System)(Phases_List, Modes_list, Order);

	// AYIN_SYSTEM(RenderSystem, AYIN_SYSTEMPHASE_LIST(Update, PostUpdate), AYIN_SCENEMODE_LIST(Editor, Runtime), 100)


};
