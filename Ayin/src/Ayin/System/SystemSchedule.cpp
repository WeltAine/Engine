#include<AyinPch.h>

#include "Ayin/System/SystemPipeline.h"
#include "Ayin/System/SystemSchedule.h"
#include "Ayin/System/SystemRegistry.h"

namespace Ayin {

	SchedulePhase& SchedulePhase::AddSystem(const PhaseSystemEntry& phaseSystemEntry) {	//! “缩写函数模板”或“简写函数模板”

		auto it = std::ranges::find_if(m_OrderedSystems, 
			[&phaseSystemEntry](const PhaseSystemEntry& entry) -> bool {
				return entry.RuntimeId == phaseSystemEntry.RuntimeId;
			});

		if (it != m_OrderedSystems.end())
			return *this;

		m_OrderedSystems.insert(phaseSystemEntry);
		return *this;

	};

	SchedulePhase& SchedulePhase::RemoveSystem(const SystemID systemId) {

		auto it = std::ranges::find_if(m_OrderedSystems,
			[&systemId](const PhaseSystemEntry& entry) -> bool {return entry.RuntimeId == systemId; }
		);

		if (it != m_OrderedSystems.end())
			m_OrderedSystems.erase(it);

		return *this;

	};

};



namespace Ayin {


	SystemSchedule::~SystemSchedule() {

		Clear();

	};


	SystemSchedule& SystemSchedule::operator=(SystemSchedule&& other) noexcept {

		if (this == &other)
			return *this;

		Clear();

		m_Systems = std::move(other.m_Systems);
		m_BegunSystems = std::move(other.m_BegunSystems);
		m_NextOrder = other.m_NextOrder;
		m_PreUpdate_Phase = std::move(other.m_PreUpdate_Phase);
		m_Update_Phase = std::move(other.m_Update_Phase);
		m_PostUpdate_Phase = std::move(other.m_PostUpdate_Phase);
		m_Presentation_Phase = std::move(other.m_Presentation_Phase);

		other.m_NextOrder = 0;
		other.m_BegunSystems.clear();

		return *this;

	};



	void SystemSchedule::Begin(const SystemContext& systemContext) {

		m_BegunSystems.clear();

		std::vector<SystemEntry*> runnableSystems;
		for (SystemEntry& entry : m_Systems) {
			if (static_cast<bool>(entry.Specification.ModeMask & systemContext.Mode))
				runnableSystems.emplace_back(&entry);
		}

		// 相同 Order 保留注册顺序；不改变 m_Systems 自身的所有权和注册顺序。
		std::stable_sort(
			runnableSystems.begin(), runnableSystems.end(),
			[](const SystemEntry* leftEntry, const SystemEntry* rightEntry) -> bool {
				return leftEntry->Specification.Order < rightEntry->Specification.Order;
			});

		for (SystemEntry* entry : runnableSystems) {
			m_BegunSystems.emplace_back(entry->Information.RuntimeId);
			entry->Instance->OnBegin(systemContext);
		}

	};



	void SystemSchedule::Run(const SystemContext& context) {

		SystemContext phaseContext = context;


		auto prePhase_Update = [&phaseContext](SchedulePhase& phase) -> void {

			std::ranges::for_each(phase, [&phaseContext](const PhaseSystemEntry& entry) -> void
				{
					const_cast<PhaseSystemEntry&>(entry).OnPreUpdate(phaseContext);
				});

			};

		auto updatePhase_Update = [&phaseContext](SchedulePhase& phase) -> void {

			std::ranges::for_each(phase, [&phaseContext](const PhaseSystemEntry& entry) -> void
				{
					const_cast<PhaseSystemEntry&>(entry).OnUpdate(phaseContext);
				});

			};

		auto postPhase_Update = [&phaseContext](SchedulePhase& phase) -> void {

			std::ranges::for_each(phase, [&phaseContext](const PhaseSystemEntry& entry) -> void
				{
					const_cast<PhaseSystemEntry&>(entry).OnPostUpdate(phaseContext);
				});

			};

		auto presentationPhase_Update = [&phaseContext](SchedulePhase& phase) -> void {

			std::ranges::for_each(phase, [&phaseContext](const PhaseSystemEntry& entry) -> void
				{
					const_cast<PhaseSystemEntry&>(entry).OnPresentationUpdate(phaseContext);
				});

			};




		phaseContext.Phase = SystemPhase::PreUpdate;
		prePhase_Update(m_PreUpdate_Phase);

		phaseContext.Phase = SystemPhase::Update;
		updatePhase_Update(m_Update_Phase);

		phaseContext.Phase = SystemPhase::PostUpdate;
		postPhase_Update(m_PostUpdate_Phase);

		phaseContext.Phase = SystemPhase::Presentation;
		presentationPhase_Update(m_Presentation_Phase);

	}


	void SystemSchedule::End(const SystemContext& systemContext) {

		// 与 Begin 相反的顺序结束，只处理本次真正执行过 OnBegin 的系统。
		std::ranges::for_each(
			m_BegunSystems | std::views::reverse,
			[this, &systemContext](const SystemID& systemId) -> void {

				auto entry = this->FindSystem(systemId);
				if (entry != this->m_Systems.end())
					entry->Instance->OnEnd(systemContext);

			});

		m_BegunSystems.clear();

	};


	SystemSchedule& SystemSchedule::AddSystem(const SystemRegistration& systemRegistration) {
	
		// 获取描述符（正确的系统描述）
		const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(systemRegistration.Information.RuntimeId);
		if (descriptor == nullptr)
			descriptor = SystemRegistry::GetSystemDescriptor(systemRegistration.Information.Name);

		if (descriptor == nullptr) {
			AYIN_CORE_ERROR("System '{}' is not registered", systemRegistration.Information.Name);
			return *this;
		}

		// 检查是否已经存在
		if (FindSystem(descriptor->Information.RuntimeId) != m_Systems.end())
			return *this;

		const int order = systemRegistration.Specification.Order < 0
			? m_NextOrder
			: systemRegistration.Specification.Order;

		// 构建实例和反序列化
		Scope<ISystem> instance = descriptor->CreateSystem();
		if (!instance) {
			AYIN_CORE_ERROR("Failed to create system '{}'", descriptor->Information.Name);
			return *this;
		}

		if (systemRegistration.SystemData.str != SystemRegistration::NullSystemData && !descriptor->DeserializeSystem(instance, systemRegistration.SystemData.str)) {
			AYIN_CORE_ERROR("Failed to deserialize system '{}'", descriptor->Information.Name);
			return *this;
		}

		// 插入系统
		SystemEntry& entry = m_Systems.emplace_back(
			SystemEntry{
				.Information{.RuntimeId{descriptor->Information.RuntimeId}, .Name{descriptor->Information.Name}},
				.Specification{.PhaseMask{systemRegistration.Specification.PhaseMask}, .ModeMask{systemRegistration.Specification.ModeMask}, .Order{order}},
				.Instance{std::move(instance)},
			});

		entry.Instance->OnAttach();
		m_NextOrder = std::max(m_NextOrder, order + 1);

		// 阶段编辑
		PhaseSystemEntry phaseEntry = static_cast<PhaseSystemEntry>(entry);
		InsertSystemToPhase(phaseEntry, Disassemble<SystemPhase>(entry.Specification.PhaseMask));

		return *this;

	};



	std::vector<SystemEntry>::iterator SystemSchedule::FindSystem(SystemID systemId) {

		auto it = std::ranges::find_if(
			m_Systems,
			[&systemId](const SystemEntry& entry) ->bool {
				return entry.Information.RuntimeId == systemId;
			}
		);

		return it;
	};

	std::vector<SystemEntry>::const_iterator SystemSchedule::FindSystem(SystemID systemId) const {

		auto it = std::ranges::find_if(
			m_Systems,
			[&systemId](const SystemEntry& entry)->bool {
				return entry.Information.RuntimeId == systemId;
			}
		);

		return it;
	};

	void SystemSchedule::InsertSystemToPhase(const PhaseSystemEntry& phaseSystemEntry, const std::vector<SystemPhase>& phases) {

		for (const SystemPhase& phase : phases) {
			switch (phase) {

			case(SystemPhase::PreUpdate): m_PreUpdate_Phase.AddSystem(phaseSystemEntry); break;
			case(SystemPhase::Update): m_Update_Phase.AddSystem(phaseSystemEntry); break;
			case(SystemPhase::PostUpdate): m_PostUpdate_Phase.AddSystem(phaseSystemEntry); break;
			case(SystemPhase::Presentation): m_Presentation_Phase.AddSystem(phaseSystemEntry); break;
			default: break;

			}
		}

	};





	void SystemSchedule::Clear() {

		// OnAttach 按注册顺序执行，因此清理时以相反顺序解除系统。
		std::ranges::for_each(
			m_Systems | std::views::reverse,
			[](const SystemEntry& entry) -> void {
				if (entry.Instance)
					entry.Instance->OnDetach();
			});

		m_Systems.clear();
		m_BegunSystems.clear();
		m_NextOrder = 0;
		m_PreUpdate_Phase = {};
		m_Update_Phase = {};
		m_PostUpdate_Phase = {};
		m_Presentation_Phase = {};

	};
	SystemEntry* SystemSchedule::FindSystemEntry(const SystemID systemId) {

		auto it = FindSystem(systemId);
		return it == m_Systems.end() ? nullptr : &*it;

	};
	const SystemEntry* SystemSchedule::FindSystemEntry(const SystemID systemId) const {

		auto it = FindSystem(systemId);
		return it == m_Systems.end() ? nullptr : &*it;

	};
	SystemEntry* SystemSchedule::FindSystemEntry(const std::string_view systemName) {

		auto it = std::ranges::find_if(
			m_Systems,
			[systemName](const SystemEntry& entry) -> bool {
				return entry.Information.Name == systemName;
			});
		return it == m_Systems.end() ? nullptr : &*it;

	};
	const SystemEntry* SystemSchedule::FindSystemEntry(const std::string_view systemName) const {

		auto it = std::ranges::find_if(
			m_Systems,
			[systemName](const SystemEntry& entry) -> bool {
				return entry.Information.Name == systemName;
			});
		return it == m_Systems.end() ? nullptr : &*it;

	};



};
