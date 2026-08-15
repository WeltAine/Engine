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
		else if ( !ScriptData.str.empty() && ScriptData.str != NullScriptData ) {
			return ScriptData;
		}

		return glz::raw_json{ NullScriptData };
	}

	void NativeScriptComponent::read_ScriptData(glz::raw_json json) {
		ScriptData = json.str.empty() ? glz::raw_json{ NullScriptData } : std::move(json);
	}


}
