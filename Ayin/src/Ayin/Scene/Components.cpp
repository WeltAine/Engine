#include<AyinPch.h>

#include "Ayin/Scene/Components.h"

#include "Ayin/Scene/ScriptRegistry.h"

#include <glaze/glaze.hpp>



namespace Ayin {
	

	//--------------------------- NativeScriptComponent -----------------------------

	glz::raw_json NativeScriptComponent::write_ScriptData() {
		if (ScriptableInstance != nullptr) {
			return glz::raw_json{ ScriptRegistry::SerializeScriptByScriptName(*this, ScriptName) };
		}

		return glz::raw_json{ NullScriptData };
	}

	void NativeScriptComponent::read_ScriptData(glz::raw_json json) {
		ScriptData = json.str.empty() ? glz::raw_json{ NullScriptData } : std::move(json);

		if (!ScriptRegistry::BindScriptByScriptName(*this, ScriptName)) {
			AYIN_CORE_WARN("Unknown script type: {}", ScriptName);
		}
		//构造也是生命周期的一部分，所以我们不在这里处理
	}


}
