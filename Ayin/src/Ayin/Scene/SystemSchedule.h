#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Scene/Scene.h"

#include "Ayin/Scene/Systems.h"

#include <queue>

//! 暂时只是 Scene 存储各个系统，没有一个更上层的东西，要用这个文件还需要解决系统生命周期管理的问题，用 Ref ？或者是单例？
 

namespace Ayin{

	enum class SystemPhase : uint8_t {

		PreUpdate = BIT(0),
		Update = BIT(1),
		PostUpdate = BIT(2),
		Presentation = BIT(3)

	};


	struct SystemContext {

		Scene& Scene;									//当前更新的场景

		Timestep DeltaTime = {};						//时间间隔

		SceneMode Mode = SceneMode::None;				//场景运行模式

		SystemPhase Phase = SystemPhase::PreUpdate;		//当前阶段

		void* UseDate = nullptr;						//用户需要特殊使用的数据（思路来源于 GLFW 中的 UsePoint）

	};



	// ------------------------------------------------------------------------------------------------------------------------

	// System 存储结构

	struct PhaseSystemEntry {

		Systems::SystemType Type;		//! 类型标志
		Systems::ISystem* Instance;		//! 观测式系统实例

		SceneMode ModeMask = SceneMode::None;	//可运行的模式

	};

	class PhaseSystems {

	public:

		inline void Run(const SystemContext& context) {
		
			std::ranges::for_each(m_PhaseSystemsQueue,
				[&context](PhaseSystemEntry& entry) {
					if ((uint8_t)context.Mode & (uint8_t)entry.ModeMask)
						entry.Instance->OnUpdate(context); 
				});

		}

		inline PhaseSystems& AddSystem(PhaseSystemEntry phaseSystemEntry) {	//! “缩写函数模板”或“简写函数模板”

			for (auto& _phaseSystemEntry : m_PhaseSystemsQueue) {
				if (_phaseSystemEntry.Type == phaseSystemEntry.Type)
					return *this;
			}

			m_PhaseSystemsQueue.push_back(phaseSystemEntry);
			return *this;

		};


		template<typename System>
			requires std::derived_from<System, Systems::ISystem>
		inline PhaseSystems& RemoveSystem() {
						
			auto it = FindSystem<SystemType>();

			if(it != m_PhaseSystemsQueue.end())
				m_PhaseSystemsQueue.erase(it);

		};

		inline std::vector<PhaseSystemEntry>::iterator begin() { return m_PhaseSystemsQueue.begin(); };
		inline std::vector<PhaseSystemEntry>::iterator end() { return m_PhaseSystemsQueue.end(); };
		inline std::vector<PhaseSystemEntry>::const_iterator begin() const { return m_PhaseSystemsQueue.begin(); };
		inline std::vector<PhaseSystemEntry>::const_iterator end() const { return m_PhaseSystemsQueue.end(); };

		inline std::vector<PhaseSystemEntry>::reverse_iterator rbegin() { return m_PhaseSystemsQueue.rbegin(); };
		inline std::vector<PhaseSystemEntry>::reverse_iterator rend() { return m_PhaseSystemsQueue.rend(); };
		inline std::vector<PhaseSystemEntry>::const_reverse_iterator rbegin() const { return m_PhaseSystemsQueue.rbegin(); };
		inline std::vector<PhaseSystemEntry>::const_reverse_iterator rend() const { return m_PhaseSystemsQueue.rend(); };



	private:
		template<typename System>
			requires std::derived_from<System, Systems::ISystem>
		inline std::vector<PhaseSystemEntry>::iterator FindSystem() {
			
			auto it = std::ranges::find_first_of(
				m_PhaseSystemsQueue.begin(), m_PhaseSystemsQueue.end(),
				[](const PhaseSystemEntry& entry)->bool {
					return entry.Type == Systems::GetSystemType<SystemType>();
				}
			);

			return it;
		};

		template<typename System>
			requires std::derived_from<System, Systems::ISystem>
		inline std::vector<PhaseSystemEntry>::const_iterator FindSystem() const {

			auto it = std::ranges::find_first_of(
				m_PhaseSystemsQueue.begin(), m_PhaseSystemsQueue.end(),
				[](const PhaseSystemEntry& entry)->bool {
					return entry.Type == Systems::GetSystemType<SystemType>();
				}
			);

			return it;
		};


	private:
		std::vector<PhaseSystemEntry> m_PhaseSystemsQueue;

	};


	// -----------------------------------------------------------------------------------------------------------------------------



	struct SystemEntry {

		Systems::SystemType Type;				//! 类型标志
		Scope<Systems::ISystem> Instance;		//! 拥有式系统实例

	};


	class SystemSchedule {

	public:

		inline void Run(const SystemContext& context) {
		
			m_PreUpdate_Phase.Run(context);
			m_Update_Phase.Run(context);
			m_PostUpdate_Phase.Run(context);
			m_Presentation_Phase.Run(context);

		}


		template<typename System>
			requires std::derived_from<System, Systems::ISystem>&& std::default_initializable<System>
		inline SystemSchedule& AddSystem(std::same_as<SystemPhase> auto... phases, std::same_as<SceneMode> auto... modes) {	//! “缩写函数模板”或“简写函数模板”

			auto it = FindSystem<System>();

			if (it != m_Systems.end())
				return *this;

			//! 系统插入
			m_Systems.emplace_back(SystemEntry{ .Type{Systems::GetSystemType<SystemType>()}, .Instance{CreateScope<SystemType>()} });
			m_Systems.back().Instance->OnAttach();

			//! 阶段编辑
			PhaseSystemEntry phaseSystemEntry{ .Type{Systems::GetSystemType<SystemType>()}, .Instance{m_Systems.back().Instance.get()}, .ModeMask{SceneMode::None | ... | modes} }

			InsertSystemToPhase(phaseSystemEntry, phases);


			return *this;

		}

		template<typename System>
			requires std::derived_from<System, Systems::ISystem>&& std::default_initializable<System>
		inline SystemSchedule& RemoveSystem() { 
			
			m_PreUpdate_Phase.RemoveSystem<System>();
			m_Update_Phase.RemoveSystem<System>();
			m_PostUpdate_Phase.RemoveSystem<System>();
			m_Presentation_Phase.RemoveSystem<System>();

			
			auto it = FindSystem<System>();
			if (it != m_Systems.end()) {
				it->OnDetach();
				m_Systems.erase(it);
			}

			return *this;
		
		};



	private:

		template<typename System>
			requires std::derived_from<System, Systems::ISystem>
		inline std::vector<SystemEntry>::iterator FindSystem() {

			auto it = std::ranges::find_first_of(
				m_Systems.begin(), m_Systems.end(),
				[](const SystemEntry& entry)->bool {
					return entry.Type == Systems::GetSystemType<SystemType>();
				}
			);

			return it;
		};

		template<typename System>
			requires std::derived_from<System, Systems::ISystem>
		inline std::vector<SystemEntry>::const_iterator FindSystem() const {

			auto it = std::ranges::find_first_of(
				m_Systems.begin(), m_Systems.end(),
				[](const PhaseSystemEntry& entry)->bool {
					return entry.Type == Systems::GetSystemType<SystemType>();
				}
			);

			return it;
		};


	private:

		inline void InsertSystemToPhase(PhaseSystemEntry phaseSystemEntry, std::same_as<SystemPhase> auto... phases) {

			switch (phases) {

				case(SystemPhase::PreUpdate): m_PreUpdate_Phase.AddSystem(phaseSystemEntry); continue;
				case(SystemPhase::Update): m_Update_Phase.AddSystem(phaseSystemEntry); continue;
				case(SystemPhase::PostUpdate): m_PostUpdate_Phase.AddSystem(phaseSystemEntry); continue;
				case(SystemPhase::Presentation): m_Presentation_Phase.AddSystem(phaseSystemEntry); continue;

			}...;

		}


	private:

		std::vector<SystemEntry> m_Systems;

		PhaseSystems m_PreUpdate_Phase;
		PhaseSystems m_Update_Phase;
		PhaseSystems m_PostUpdate_Phase;
		PhaseSystems m_Presentation_Phase;

		};

	};
