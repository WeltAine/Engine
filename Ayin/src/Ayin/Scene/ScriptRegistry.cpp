#include <AyinPch.h>

#include "Ayin/Scene/ScriptRegistry.h"

#include <algorithm>

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

		if (scriptName.empty() || scriptName == NativeScriptComponent::NoneScriptName) {
			return NativeScriptComponent::NullScriptData;
		}
	
		auto desc = ScriptRegistry::GetScriptDescriptor(scriptName);
		
		if (desc.has_value())
			return (*desc).serializeScript(nsc);
		else
			return NativeScriptComponent::NullScriptData;

	};

	bool ScriptRegistry::DeserializeScriptByScriptName(NativeScriptComponent& nsc, const std::string& scriptName, const std::string& json) {

		if (scriptName.empty() || scriptName == NativeScriptComponent::NoneScriptName) {
			return true;
		}
		
		auto desc = ScriptRegistry::GetScriptDescriptor(scriptName);

		if(desc.has_value())
			return (*desc).deserializeScript(nsc, json);

		return false;
	};

	bool ScriptRegistry::BindScriptByScriptName(NativeScriptComponent& nsc, const std::string& scriptName) {

		if (scriptName.empty() || scriptName == NativeScriptComponent::NoneScriptName) {// 没脚本
			return false;
		}
		
		auto desc = ScriptRegistry::GetScriptDescriptor(scriptName);

		if(desc.has_value()) {
			(*desc).bindScript(nsc);
			return true;
		}

		return false;

	};

	std::vector<std::string> ScriptRegistry::GetAvailableScriptNames() {

		std::vector<std::string> scriptNames;
		auto& allScriptDescriptors = GetAllScriptDescriptors();
		scriptNames.reserve(allScriptDescriptors.size());

		for (auto&& [scriptName, _] : allScriptDescriptors) {
			scriptNames.push_back(scriptName);
		}

		std::sort(scriptNames.begin(), scriptNames.end());

		return scriptNames;

	}


}
