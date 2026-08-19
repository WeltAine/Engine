#include <AyinPch.h>

#include "Ayin/System/SystemRegistry.h"


namespace Ayin {


	Scope<ISystem> SystemRegistry::CreateSystemBy(const std::string_view systemName) {
		
		std::optional<SystemDescriptor> systemDescriptor = GetSystemDescriptor(systemName);

		if (!systemDescriptor)
			return nullptr;

		return (*systemDescriptor).CreateSystem();
	
	};
	Scope<ISystem> SystemRegistry::CreateSystemBy(SystemID systemId) {

		std::optional<SystemDescriptor> systemDescriptor = GetSystemDescriptor(systemId);

		if (!systemDescriptor)
			return nullptr;

		return (*systemDescriptor).CreateSystem();

	};

	std::string SystemRegistry::SerializeSystem(const Scope<ISystem>& system, std::string_view systemName) {
		
		std::optional<SystemDescriptor> systemDescriptor = GetSystemDescriptor(systemName);

		if (!systemDescriptor)
			return SystemRegistration::NullSystemData;

		return systemDescriptor->SerializeSystem(system);

	};

	bool SystemRegistry::DeserializeSystem(Scope<ISystem>& system, std::string_view systemName, const std::string& json) {

		std::optional<SystemDescriptor> systemDescriptor = GetSystemDescriptor(systemName);

		if (!systemDescriptor)
			return SystemRegistration::NullSystemData;

		return systemDescriptor->DeserializeSystem(system, json);

	};

	std::vector<SystemDescriptor>& SystemRegistry::GetAllSystemDescriptors() {
	
		static std::vector<SystemDescriptor> systemDescriptors;

		return systemDescriptors;

	};
	std::optional<SystemDescriptor&> SystemRegistry::GetSystemDescriptor(std::string_view systemName) {
	
		auto it = std::ranges::find_if(
			GetAllSystemDescriptors(),
			[&systemName](const SystemDescriptor& descriptor) -> bool {
				
				return descriptor.Information.Name == systemName;

			});

		if (it != GetAllSystemDescriptors().end())
			return *it;

		return std::nullopt;

	};
	std::optional<SystemDescriptor&> SystemRegistry::GetSystemDescriptor(SystemID systemId) {
	
		auto it = std::ranges::find_if(
			GetAllSystemDescriptors(),
			[&systemId](const SystemDescriptor& descriptor) -> bool {

				return descriptor.Information.RuntimeId == systemId;

			});

		if (it != GetAllSystemDescriptors().end())
			return *it;

		return std::nullopt;

	};

};
