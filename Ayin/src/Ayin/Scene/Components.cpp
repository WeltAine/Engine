#include<AyinPch.h>

#include "Ayin/Scene/Components.h"

#include "Ayin/Scene/ScriptRegistry.h"

#include <glaze/glaze.hpp>



namespace Ayin {
	

	//--------------------------- NativeScriptComponent -----------------------------

	std::string NativeScriptComponent::write_ScriptData() {
		if (ScriptableInstance != nullptr) {
			return ScriptRegistry::SerializeScriptByScriptName(*this, ScriptName);
		}
		else {
			return "{}";
		}
	}

	void NativeScriptComponent::read_ScriptData(const std::string& json) {
		ScriptRegistry::BindScriptByScriptName(*this, ScriptName);
		ScriptData = glz::raw_json(json);
		//构造也是生命周期的一部分，所以我们不在这里处理
	}


}
