#include <AyinPch.h>

#include "Ayin/Scene/ScriptRegistry.h"

namespace Ayin {


	std::unordered_map<std::string, ScriptDescriptor>& ScriptRegistry::GetAllScriptDescriptors() {
	
		static std::unordered_map<std::string, ScriptDescriptor> scriptDescriptors;

		return scriptDescriptors;

	};

	std::optional<ScriptDescriptor> ScriptRegistry::GetScriptDescriptor(const std::string& scriptName) {
		
		auto& allScriptDescriptors = GetAllScriptDescriptors();
		
		if(allScriptDescriptors.contains(scriptName))
			return { allScriptDescriptors[scriptName] };

		return std::nullopt;

	};


	std::string ScriptRegistry::SerializeScriptByScriptName(NativeScriptComponent& nsc, const std::string& scriptName) {
	
		auto desc = ScriptRegistry::GetScriptDescriptor(scriptName);
		
		if (desc.has_value())
			return (*desc).serializeScript(nsc);
		else
			return "{}";

	};

	void ScriptRegistry::DeserializeScriptByScriptName(NativeScriptComponent& nsc, const std::string& scriptName, const std::string& json) {
		
		auto desc = ScriptRegistry::GetScriptDescriptor(scriptName);

		if(desc.has_value())
			(*desc).deserializeScript(nsc, json);
	};

	void ScriptRegistry::BindScriptByScriptName(NativeScriptComponent& nsc, const std::string& scriptName) {
		
		auto desc = ScriptRegistry::GetScriptDescriptor(scriptName);

		if(desc.has_value())
			(*desc).bindScript(nsc);

	};


}
