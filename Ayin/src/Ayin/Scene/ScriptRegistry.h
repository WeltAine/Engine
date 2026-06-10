#pragma once
#include "Ayin/Core/Core.h"
#include "Ayin/Scene/Components.h"

#include <entt/entt.hpp>
#include <glaze/glaze.hpp>
#include <functional>
#include <unordered_map>
#include <optional>
#include <string>
#include <vector>



namespace Ayin {

	template<typename T>
	concept HasScriptGlazeMeta = requires { T::glaze::value; };

	struct AYIN_API ScriptDescriptor {

		entt::id_type ScrpitID;

		std::string ScriptName;
		std::function<std::string(NativeScriptComponent&)> serializeScript;
		std::function<bool(NativeScriptComponent&, const std::string&)> deserializeScript;
		std::function<void(NativeScriptComponent&)> bindScript;
		
	};

	class AYIN_API ScriptRegistry {

	public:

		//注册脚本
		template<typename ScriptType>
			requires std::derived_from<ScriptType, ScriptableEntity>&& std::default_initializable<ScriptType>
		static void Registry(const std::string& name);

		//NSC.ScriptInstance <-> NSC.ScriptData
		//仅脚本序列化为NSC中的raw_json
		//不想使用多态，那么这就是必要的，否则你无法在运行时，在无法写出具体类型时，精准完成对应序列化，比如你手上握着的是一个指向派生类的基类指针而已
		static std::string SerializeScriptByScriptName(NativeScriptComponent& nsc, const std::string& scriptName);
		//存在理由同上
		static bool DeserializeScriptByScriptName(NativeScriptComponent& nsc, const std::string& scriptName, const std::string& json);

		static bool BindScriptByScriptName(NativeScriptComponent& nsc, const std::string& scriptName);

		static std::vector<std::string> GetAvailableScriptNames();


	private:
		static std::unordered_map<std::string, ScriptDescriptor>& GetAllScriptDescriptors();

		static std::optional<ScriptDescriptor> GetScriptDescriptor(const ::std::string& scriptName);


	};


	//-------------------  辅助方法 ---------------------
	template<typename ScriptType>
		requires std::derived_from<ScriptType, ScriptableEntity>&& std::default_initializable<ScriptType>
	std::string SerializeScript(const NativeScriptComponent& nsc) {

		if (nsc.ScriptableInstance == nullptr) {
			return NativeScriptComponent::NullScriptData;
		}

		ScriptType* script = static_cast<ScriptType*>(nsc.ScriptableInstance);

		if constexpr (HasScriptGlazeMeta<ScriptType>) {
			auto result = glz::write_json(*script);
			if (!result) {
				return "{}";
			}
			else {
				return std::move(*result);
			}
		}
		else {
			return "{}";
		}

	}

	 template<typename ScriptType>
		 requires std::derived_from<ScriptType, ScriptableEntity>&& std::default_initializable<ScriptType>
	 bool DeserializeScript(NativeScriptComponent& nsc, const std::string& json) {

		 if (nsc.ScriptableInstance == nullptr || json.empty() || json == NativeScriptComponent::NullScriptData) {//有反序列化对象，有反序列化数据，不用反序列化
			 return true;
		 }
		
		 if constexpr (!HasScriptGlazeMeta<ScriptType>) {//脚本本身没有序列化
			 return true;
		 }
		 else {
			 ScriptType* script = static_cast<ScriptType*>(nsc.ScriptableInstance);

			 auto err = glz::read_json(*script, json);

			 if (err) {
				 AYIN_CORE_ERROR("Deserialize {} failed: {}", nsc.ScriptName, glz::format_error(err, json));
				 return false;
			 }

			 return true;
		 }

	 }
	 //--------------------------------------------------


	//注册脚本活动信息
	template<typename ScriptType>
		requires std::derived_from<ScriptType, ScriptableEntity>&& std::default_initializable<ScriptType>
	void ScriptRegistry::Registry(const std::string& scriptName) {

		ScriptDescriptor scriptDescriptor;

		scriptDescriptor.ScrpitID = entt::type_hash<ScriptType>().value();
		scriptDescriptor.ScriptName = scriptName;

		scriptDescriptor.serializeScript = &SerializeScript<ScriptType>;

		scriptDescriptor.deserializeScript = &DeserializeScript<ScriptType>;

		scriptDescriptor.bindScript = [](NativeScriptComponent& nsc) { nsc.Bind<ScriptType>(); };
	
		auto& allScriptDescriptors = GetAllScriptDescriptors();
		allScriptDescriptors[scriptDescriptor.ScriptName] = scriptDescriptor;

	};

	namespace detail {

		template<typename ScriptType>
		struct ScriptRegistrar {

			ScriptRegistrar(const std::string& scriptName) {
				::Ayin::ScriptRegistry::Registry<ScriptType>(scriptName);
			}

		};

	}

	#define AYIN_SCRIPT(ScriptType) \
		inline static ::Ayin::detail::ScriptRegistrar<ScriptType> AYIN_CONCAT(_reg_, ScriptType)(#ScriptType);\


}
