#include <AyinPch.h>

#include "Ayin/System/SystemRegistry.h"


namespace Ayin {

	Scope<ISystem> SystemRegistry::CreateSystemBy(const std::string_view typeKey) {

		const SystemDescriptor* descriptor = GetSystemDescriptor(typeKey);
		if (descriptor == nullptr) {
			return nullptr;
		}

		try {
			return descriptor->Create();
		}
		catch (const std::exception& exception) {
			AYIN_CORE_ERROR("Failed to create System '{}': {}", typeKey, exception.what());
		}
		catch (...) {
			AYIN_CORE_ERROR("Failed to create System '{}': unknown exception", typeKey);
		}

		return nullptr;

	};

	Scope<ISystem> SystemRegistry::CreateSystemBy(const SystemID runtimeId) {

		const SystemDescriptor* descriptor = GetSystemDescriptor(runtimeId);
		if (descriptor == nullptr) {
			return nullptr;
		}

		return CreateSystemBy(descriptor->TypeKey);

	};


	SerializeSystemConfigurationResult SystemRegistry::SerializeConfiguration(const ISystem& system, const std::string_view typeKey) {

		const SystemDescriptor* descriptor = GetSystemDescriptor(typeKey);
		if (descriptor == nullptr) {
			return {.Error{fmt::format("System '{}' is not registered", typeKey)}};
		}

		try {
			return descriptor->SerializeConfiguration(system);
		}
		catch (const std::exception& exception) {
			return {.Error{exception.what()}};
		}
		catch (...) {
			return {.Error{"unknown exception while serializing System configuration"}};
		}

	};

	SerializeSystemConfigurationResult SystemRegistry::SerializeConfiguration(const ISystem& system, const SystemID runtimeId) {

		const SystemDescriptor* descriptor = GetSystemDescriptor(runtimeId);
		if (descriptor == nullptr) {
			return {.Error{"System RuntimeId is not registered"}};
		}

		return SerializeConfiguration(system, descriptor->TypeKey);

	};


	DeserializeSystemConfigurationResult SystemRegistry::DeserializeConfiguration(
		ISystem& system,
		const std::string_view typeKey,
		const std::string_view json) {

		const SystemDescriptor* descriptor = GetSystemDescriptor(typeKey);
		if (descriptor == nullptr) {
			return {.Error{fmt::format("System '{}' is not registered", typeKey)}};
		}

		try {
			return descriptor->DeserializeConfiguration(system, json);
		}
		catch (const std::exception& exception) {
			return {.Error{exception.what()}};
		}
		catch (...) {
			return {.Error{"unknown exception while deserializing System configuration"}};
		}

	};

	DeserializeSystemConfigurationResult SystemRegistry::DeserializeConfiguration(
		ISystem& system,
		const SystemID runtimeId,
		const std::string_view json) {

		const SystemDescriptor* descriptor = GetSystemDescriptor(runtimeId);
		if (descriptor == nullptr) {
			return {.Error{"System RuntimeId is not registered"}};
		}

		return DeserializeConfiguration(system, descriptor->TypeKey, json);

	};


	const std::vector<SystemDescriptor>& SystemRegistry::GetAllSystemDescriptors() {

		return GetAllSystemDescriptorsMutable();

	};

	const SystemDescriptor* SystemRegistry::GetSystemDescriptor(const std::string_view typeKey) {

		const auto& descriptors = GetAllSystemDescriptors();
		auto it = std::ranges::find_if(
			descriptors,
			[typeKey](const SystemDescriptor& descriptor) -> bool {
				return descriptor.TypeKey == typeKey;
			});

		return it == descriptors.end() ? nullptr : &*it;

	};

	const SystemDescriptor* SystemRegistry::GetSystemDescriptor(const SystemID runtimeId) {

		const auto& descriptors = GetAllSystemDescriptors();
		auto it = std::ranges::find_if(
			descriptors,
			[runtimeId](const SystemDescriptor& descriptor) -> bool {
				return descriptor.RuntimeId == runtimeId;
			});

		return it == descriptors.end() ? nullptr : &*it;

	};


	std::vector<SystemDescriptor>& SystemRegistry::GetAllSystemDescriptorsMutable() {

		static std::vector<SystemDescriptor> systemDescriptors;

		return systemDescriptors;

	};


};
