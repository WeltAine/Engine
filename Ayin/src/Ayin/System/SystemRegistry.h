#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/System/SystemTypes.h"

#include <glaze/glaze.hpp>

#include <concepts>
#include <functional>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <vector>


namespace Ayin {


	struct SerializeSystemConfigurationResult {

		std::string Json = "{}";
		std::string Error;

		[[nodiscard]] explicit operator bool() const { return Error.empty(); }

	};

	struct DeserializeSystemConfigurationResult {

		std::string Error;

		[[nodiscard]] explicit operator bool() const { return Error.empty(); }

	};


	// 系统是否具备独立的序列化要求。
	template<typename System>
	concept HasSystemGlazeMeta = requires { System::glaze::value; };


	struct AYIN_API SystemDescriptor {

		SystemID RuntimeId;
		SystemTypeKey TypeKey;
		std::string DisplayName;
		SystemSpecification DefaultSpecification;

		std::function<Scope<ISystem>()> Create;
		std::function<SerializeSystemConfigurationResult(const ISystem&)> SerializeConfiguration;
		std::function<DeserializeSystemConfigurationResult(ISystem&, std::string_view)> DeserializeConfiguration;

	};


	class AYIN_API SystemRegistry {

	public:

		template<typename System>
			requires std::derived_from<System, ISystem>&& std::default_initializable<System>
		static bool Register(
			std::string_view typeKey,
			std::string_view displayName,
			const std::initializer_list<SystemPhase>& defaultPhases,
			const std::initializer_list<SceneMode>& defaultModes,
			int order);

		static Scope<ISystem> CreateSystemBy(std::string_view typeKey);
		static Scope<ISystem> CreateSystemBy(SystemID runtimeId);

		static SerializeSystemConfigurationResult SerializeConfiguration(const ISystem& system, std::string_view typeKey);
		static SerializeSystemConfigurationResult SerializeConfiguration(const ISystem& system, SystemID runtimeId);
		static DeserializeSystemConfigurationResult DeserializeConfiguration(ISystem& system, std::string_view typeKey, std::string_view json);
		static DeserializeSystemConfigurationResult DeserializeConfiguration(ISystem& system, SystemID runtimeId, std::string_view json);

		static const std::vector<SystemDescriptor>& GetAllSystemDescriptors();
		static const SystemDescriptor* GetSystemDescriptor(std::string_view typeKey);
		static const SystemDescriptor* GetSystemDescriptor(SystemID runtimeId);

	private:

		static std::vector<SystemDescriptor>& GetAllSystemDescriptorsMutable();

	};


	template<typename System>
		requires std::derived_from<System, ISystem>&& std::default_initializable<System>
	bool SystemRegistry::Register(
		const std::string_view typeKey,
		const std::string_view displayName,
		const std::initializer_list<SystemPhase>& defaultPhases,
		const std::initializer_list<SceneMode>& defaultModes,
		const int order) {

		// 空 TypeKey、重复 TypeKey 和重复 RuntimeId 都会让持久化身份失去唯一性。（简单来说就是不规范的注册）
		if (typeKey.empty()) {
			AYIN_CORE_ERROR("System registration failed: TypeKey cannot be empty");
			return false;
		}

		const SystemID runtimeId = GetSystemID<System>();
		if (GetSystemDescriptor(typeKey) != nullptr || GetSystemDescriptor(runtimeId) != nullptr) {
			AYIN_CORE_ERROR("System registration failed: duplicate TypeKey '{}' or RuntimeId", typeKey);
			return false;
		}

		SystemPhase phaseMask = SystemPhase::None;
		for (const SystemPhase phase : defaultPhases) {
			phaseMask |= phase;
		}

		SceneMode modeMask = SceneMode::None;
		for (const SceneMode mode : defaultModes) {
			modeMask |= mode;
		}

		SystemDescriptor descriptor{
			.RuntimeId{runtimeId},
			.TypeKey{typeKey},
			.DisplayName{displayName.empty() ? std::string{ typeKey } : std::string{ displayName }},
			.DefaultSpecification{
				.PhaseMask{phaseMask},
				.ModeMask{modeMask},
				.Order{order}
			},
			.Create{
				[]() -> Scope<ISystem> {
					return CreateScope<System>();
				}
			},
			.SerializeConfiguration{
				[](const ISystem& system) -> SerializeSystemConfigurationResult {
					if constexpr (HasSystemGlazeMeta<System>) {
						try {
							const System* concreteSystem = dynamic_cast<const System*>(&system);
							if (concreteSystem == nullptr) {
								return {.Error{ "System configuration type does not match its TypeKey" }};
							}

							auto result = ::glz::write_json(*concreteSystem);
							if (!result) {
								return {.Error{glz::format_error(result.error())}};
							}

							return {.Json{std::move(*result)}};
						}
						catch (const std::exception& exception) {
							return {.Error{exception.what()}};
						}
						catch (...) {
							return {.Error{"unknown exception while serializing System configuration"}};
						}
					}

					return {};
				}
			},
			.DeserializeConfiguration{
				[](ISystem& system, const std::string_view json) -> DeserializeSystemConfigurationResult {
					if constexpr (HasSystemGlazeMeta<System>) {
						try {
							System* concreteSystem = dynamic_cast<System*>(&system);
							if (concreteSystem == nullptr) {
								return {.Error{ "System configuration type does not match its TypeKey" }};
							}

							auto error = ::glz::read_json(*concreteSystem, json);
							if (error) {
								return {.Error{glz::format_error(error, json)}};
							}

							return {};
						}
						catch (const std::exception& exception) {
							return {.Error{exception.what()}};
						}
						catch (...) {
							return {.Error{"unknown exception while deserializing System configuration"}};
						}
					}

					return json.empty() || json == "{}"
						? DeserializeSystemConfigurationResult{}
						: DeserializeSystemConfigurationResult{.Error{"System does not declare serializable configuration"}};
				}
			}
		};

		GetAllSystemDescriptorsMutable().emplace_back(std::move(descriptor));
		return true;

	};


	namespace detail {

		template<typename System>
			requires std::derived_from<System, ISystem>&& std::default_initializable<System>
		struct AYIN_API SystemRegistrar {

			inline SystemRegistrar(
				const std::string_view typeKey,
				const std::string_view displayName,
				const std::initializer_list<SystemPhase>& defaultPhases,
				const std::initializer_list<SceneMode>& defaultModes,
				const int order) {

				::Ayin::SystemRegistry::Register<System>(typeKey, displayName, defaultPhases, defaultModes, order);

			}

		};
	}


#define AYIN_SYSTEMPHASE_LIST(...) \
	{ __VA_ARGS__ }

#define AYIN_SCENEMODE_LIST(...) \
	{ __VA_ARGS__ }

// System 注册定义在命名空间作用域；inline 让同一 System 的头文件注册变量在整个程序中只有一份。
// 不能在这里保留 static，否则每个包含该头文件的编译单元都会各自尝试重复注册。
#define AYIN_SYSTEM(System, TypeKey, DisplayName, Phases_List, Modes_List, Order) \
	inline ::Ayin::detail::SystemRegistrar<System> AYIN_CONCAT(_reg_, System)(TypeKey, DisplayName, Phases_List, Modes_List, Order);


};
