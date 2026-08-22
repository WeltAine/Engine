#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/SceneMode.h"
#include "Ayin/System/SystemPipeline.h"
#include "Ayin/System/SystemSchedule.h"

#include <string_view>

namespace Ayin {

	class EditorCamera;
	class Scene;

	class World {

		friend class WorldSerializer;
	
	private:

		Ref<Scene> m_ActiveScene;
		SystemSchedule m_SystemSchedule;

		SceneMode m_CurrentMode = SceneMode::None;


	public:

		World(Ref<Scene> scene, const SystemPipeline& systemPipeline);
		~World();

		// 返回 bool 表明是否真的被执行还是遭遇了异常, bool 可以显式暴露非法调用
		bool BeginWorldExecutionSession(SceneMode mode);	// 转发到 Schedule 的 Begin 对该模式下所有可以运行的 System 执行 OnBegin
		bool Update(Timestep deltaTime);
		//ToDo: 临时的重载，用于 编辑器 的编辑模式
		bool Update(Timestep deltaTime, EditorCamera* editorCamera);
		bool EndWorldExecutionSession();					// 转发到 Schedule 的 End 对已经 Begin 的系统进行
		std::vector<SystemRuntimeView> GetRuntimeViews() const { return m_SystemSchedule.GetRuntimeViews(); };

		ISystem* FindSystemInstance(SystemID systemId);
		const ISystem* FindSystemInstance(SystemID systemId) const;
		ISystem* FindSystemInstance(std::string_view systemName);
		const ISystem* FindSystemInstance(std::string_view systemName) const;

		// 在安全的 World 接口内整体替换 Schedule；失败时保持旧 Schedule 不变。
		bool ApplyPipeline(const SystemPipeline& systemPipeline);

		inline bool SessionReady() const { return m_CurrentMode != SceneMode::None && m_ActiveScene != nullptr; };

		void ResetSchedule(const SystemPipeline& systemPipeline);
	
		bool TransitionMode(SceneMode mode);

		//? 有时候我会很纠结是否开放 Get 接口，尤其是这些和生命周期很相关的东西，外部访问可能非法改变状态以及生命周期的完整性
		inline const Ref<Scene> GetScene() const { return m_ActiveScene; };
		inline Ref<Scene> GetScene() { return m_ActiveScene; };
		inline const SystemSchedule& GetSystemSchedule() const { return m_SystemSchedule; };
		inline SceneMode GetCurrentMode() const { return m_CurrentMode; };

	};


};
