#include <AyinPch.h>

#include "Ayin/System/SystemRegistry.h"
#include "Ayin/System/SystemPipeline.h"


namespace Ayin {


	SystemDescriptor::operator SystemRegistration() const {
		return SystemRegistration{
			.Information{Information},
			.Specification{DefaultSpecification}
		};
	}


	Scope<ISystem> SystemRegistry::CreateSystemBy(const std::string_view systemName) {
		
		const SystemDescriptor* systemDescriptor = GetSystemDescriptor(systemName);

		if (systemDescriptor == nullptr)
			return nullptr;

		return systemDescriptor->CreateSystem();
	
	};
	Scope<ISystem> SystemRegistry::CreateSystemBy(SystemID systemId) {

		const SystemDescriptor* systemDescriptor = GetSystemDescriptor(systemId);

		if (systemDescriptor == nullptr)
			return nullptr;

		return systemDescriptor->CreateSystem();

	};


	std::string SystemRegistry::SerializeSystem(const Scope<ISystem>& system, std::string_view systemName) {
		
		const SystemDescriptor* systemDescriptor = GetSystemDescriptor(systemName);

		if (systemDescriptor == nullptr)
			return SystemDescriptor::NullSystemData;

		return systemDescriptor->SerializeSystem(system);

	};
	std::string SystemRegistry::SerializeSystem(const Scope<ISystem>& system, const SystemID systemId) {
	
		const SystemDescriptor* systemDescriptor = GetSystemDescriptor(systemId);

		if (systemDescriptor == nullptr)
			return SystemDescriptor::NullSystemData;

		return systemDescriptor->SerializeSystem(system);

	};


	bool SystemRegistry::DeserializeSystem(Scope<ISystem>& system, std::string_view systemName, const std::string& json) {

		const SystemDescriptor* systemDescriptor = GetSystemDescriptor(systemName);

		if (systemDescriptor == nullptr)
			return false;

		return systemDescriptor->DeserializeSystem(system, json);

	};
	bool SystemRegistry::DeserializeSystem(Scope<ISystem>& system, const SystemID systemId, const std::string& json) {
		
		const SystemDescriptor* systemDescriptor = GetSystemDescriptor(systemId);

		if (systemDescriptor == nullptr)
			return false;

		return systemDescriptor->DeserializeSystem(system, json);

	};


	const std::vector<SystemDescriptor>& SystemRegistry::GetAllSystemDescriptors() {

		return GetAllSystemDescriptorsMutable();

	};
	const SystemDescriptor* SystemRegistry::GetSystemDescriptor(std::string_view systemName) {
	
		auto it = std::ranges::find_if(
			GetAllSystemDescriptors(),
			[&systemName](const SystemDescriptor& descriptor) -> bool {
				
				return descriptor.Information.Name == systemName;

			});

		if (it != GetAllSystemDescriptors().end())
			return &(*it);

		return nullptr;

	};
	const SystemDescriptor* SystemRegistry::GetSystemDescriptor(SystemID systemId) {
	
		auto it = std::ranges::find_if(
			GetAllSystemDescriptors(),
			[&systemId](const SystemDescriptor& descriptor) -> bool {

				return descriptor.Information.RuntimeId == systemId;

			});

		if (it != GetAllSystemDescriptors().end())
			return &(*it);

		return nullptr;

	};


	std::vector<SystemDescriptor>& SystemRegistry::GetAllSystemDescriptorsMutable() {
	
		static std::vector<SystemDescriptor> systemDescriptors;

		return systemDescriptors;

	};


};
