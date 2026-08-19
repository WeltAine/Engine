#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/System/SystemSchedule.h"
#include "Ayin/System/SystemPipeline.h"

namespace Ayin {

	// 系统是否具备独立的序列化要求
	template<typename System>
	concept HasSystemGlazeMate = requires{System::glaze::value; };

	
	struct AYIN_API SystemDescriptor {

		SystemInformation Information;
		SystemSpecification DefaultSpecification;

		std::function<Scope<ISystem>()> CreateSystem;
		std::function<std::string(const Scope<ISystem>&)> SerializeSystem;					// 序列化 系统的 自有数据
		std::function<bool(Scope<ISystem>&, const std::string&)> DeserializeSystem;			// 反序列化 系统改的自有数据

	};


	class AYIN_API SystemRegistry {

	public:

		template<typename System>
			requires std::derived_from<System, ISystem>&& std::default_initializable<System>
		static inline void Registry(const std::initializer_list<SystemPhase>& defaultPhases, const std::initializer_list<SceneMode>& defaultModes, int order) {
		
			// 系统信息
			SystemInformation information{
				.Id{GetSystemID<System>()},
				.Name{typeid(System).name()}
			};

			// 系统参数
			SystemPhase phaseMask = SystemPhase::None;
			for (auto phase : defaultPhases) {
				phaseMask |= phase;
			}

			SceneMode modeMask = SceneMode::None;
			for (auto mode : defaultModes) {
				modeMask |= mode;
			}

			SystemSpecification specification{
				.Order{order},
				.PhaseMask{phaseMask},
				.ModeMask{modeMask}
			};


			// 系统回调
			std::function<Scope<ISystem>()> createSystem = 
				[]() -> Scope<ISystem> {
			
					return CreateScope<System>();//！ std::unique_ptr 提供的 converting move constructor，即转换移动构造。（当模板类型具备继承关系时可以转换）

				};

			std::function<std::string(const Scope<ISystem>&)> serializeSystem =
				[](const Scope<ISystem>& system_ptr) ->std::string {
					
				if constexpr (HasSystemGlazeMate<System>) {
				
					if (system_ptr.get()) {
					
						System* system = static_cast<System*>(system_ptr.get());

						auto result = ::glz::write_json(*system);
						if (!result) {
							return SystemRegistration::NullSystemData;
						}

						return *result;
					
					}
					else {
						return SystemRegistration::NullSystemData;
					}

				}

				return SystemRegistration::NullSystemData;
				
				};

			std::function<bool(Scope<ISystem>&, const std::string&)> deserializeSystem =
				[](Scope<ISystem>& system_ptr, const std::string& json) -> bool {
				
				if constexpr (HasSystemGlazeMate<System>) {

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

				return false;

				};


			// 系统描述符封装
			std::vector<SystemDescriptor>& allSystemDescriptors = GetAllSystemDescriptors();

			allSystemDescriptors.emplace_back(
				SystemDescriptor{
					.Information{std::move(information)},
					.DefaultSpecification{std::move(specification)},
					.AddSystem{std::move(createSystem)},
					.Serialize{std::move(serializeSystem)},
					.Deserialize{std::move(deserializeSystem)}
				} 
			);
		
		};

		static Scope<ISystem> CreateSystemBy(const std::string_view systemName);
		static Scope<ISystem> CreateSystemBy(SystemID systemId);
		static std::string SerializeSystem(const Scope<ISystem>& system, std::string_view systemName);
		static bool DeserializeSystem(Scope<ISystem>& system, std::string_view systemName, const std::string& json);


		static std::vector<SystemDescriptor>& GetAllSystemDescriptors();
		static std::optional<SystemDescriptor&> GetSystemDescriptor(std::string_view systemName);
		static std::optional<SystemDescriptor&> GetSystemDescriptor(SystemID systemId);

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


//#define AYIN_SYSTEM(System) \
//		inline static ::Ayin::detail::SystemRegistrar<ScriptType> AYIN_CONCAT(_reg_, System)(#System);\


};
