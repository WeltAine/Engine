#include<AyinPch.h>

#include "Ayin/System/SystemSchedule.h"
#include "Ayin/System/SystemRegistry.h"

namespace Ayin {

	SystemSchedule::~SystemSchedule() {

		if (IsActive()) {
			AYIN_CORE_WARN("SystemSchedule was destroyed while active; World should End it before destruction");
			return;
		}

		// Schedule 只能在 Idle 状态下执行 Detach；World 会在析构或替换前主动完成这一步。
		DetachSystems();

	};

	SystemSchedule::SystemSchedule(SystemSchedule&& other) noexcept {

		MoveFrom(std::move(other));

	};

	SystemSchedule& SystemSchedule::operator=(SystemSchedule&& other) noexcept {

		if (this == &other)
			return *this;

		if (IsActive() || m_Attached || !m_AttachSequence.empty()) {
			AYIN_CORE_WARN("Replacing a live SystemSchedule; the owner should End and Detach it first");
			return *this;
		}

		MoveFrom(std::move(other));

		return *this;

	};


	void SystemSchedule::Begin(const SystemContext& systemContext) {

		if (!m_TopologySealed) {
			AYIN_CORE_WARN("SystemSchedule must be built before Begin");
			return;
		}

		if (!m_Attached) {
			AYIN_CORE_WARN("SystemSchedule must be attached before Begin");
			return;
		}

		if (IsActive()) {
			AYIN_CORE_WARN("SystemSchedule is already active");
			return;
		}

		if (systemContext.Mode == SceneMode::None) {
			AYIN_CORE_WARN("SystemSchedule can not begin with SceneMode::None");
			return;
		}


		// 开始对该模式下可以运行的 System 进行 OnBegin()，并记录已经成功 Begin 的 System
		m_BegunSystems.clear();
		m_LifecycleState = LifecycleState::Active;

		for (const SystemIndex index : m_BeginPlan) {
			SystemEntry& entry = m_Systems[index];
			if (!Contains(entry.Specification.ModeMask, systemContext.Mode))
				continue;

			try {
				entry.Instance->OnBegin(systemContext);
				m_BegunSystems.emplace_back(index);
			}
			catch (const std::exception& exception) {
				AYIN_CORE_ERROR("System '{}' failed during OnBegin: {}", entry.Information.TypeKey, exception.what());
			}
			catch (...) {
				AYIN_CORE_ERROR("System '{}' failed during OnBegin", entry.Information.TypeKey);
			}
		}

	};


	void SystemSchedule::Run(const SystemContext& context) {

		if (!IsActive()) {
			AYIN_CORE_WARN("SystemSchedule must begin before running");
			return;
		}

		SystemContext phaseContext = context;

		// 让目标阶段中的 System 执行特定回调（因为我们的系统支持同时在不同阶段运作）
													// 目标阶段				让每个阶段执行的回调
		const auto runPhase = [this, &phaseContext](const SystemPhase phase, auto callback) -> void {

			phaseContext.Phase = phase;
			for (const SystemIndex index : GetPhasePlan(phase)) {
				SystemEntry& entry = m_Systems[index];
				if (!Contains(entry.Specification.ModeMask, phaseContext.Mode))
					continue;

				try {
					callback(*entry.Instance, phaseContext);
				}
				catch (const std::exception& exception) {
					AYIN_CORE_ERROR("System '{}' failed during phase {}: {}", entry.Information.TypeKey, static_cast<int>(phase), exception.what());
				}
				catch (...) {
					AYIN_CORE_ERROR("System '{}' failed during phase {}", entry.Information.TypeKey, static_cast<int>(phase));
				}
			}

		};

		runPhase(SystemPhase::PreUpdate, [](ISystem& system, const SystemContext& phaseContext) { system.OnPreUpdate(phaseContext); });
		runPhase(SystemPhase::Update, [](ISystem& system, const SystemContext& phaseContext) { system.OnUpdate(phaseContext); });
		runPhase(SystemPhase::PostUpdate, [](ISystem& system, const SystemContext& phaseContext) { system.OnPostUpdate(phaseContext); });
		runPhase(SystemPhase::Presentation, [](ISystem& system, const SystemContext& phaseContext) { system.OnPresentationUpdate(phaseContext); });

	};


	void SystemSchedule::End(const SystemContext& systemContext) {

		if (!IsActive()) {
			AYIN_CORE_WARN("SystemSchedule is not active");
			return;
		}

		for (auto it = m_BegunSystems.rbegin(); it != m_BegunSystems.rend(); ++it) {
			SystemEntry& entry = m_Systems[*it];

			try {
				entry.Instance->OnEnd(systemContext);
			}
			catch (const std::exception& exception) {
				AYIN_CORE_ERROR("System '{}' failed during OnEnd: {}", entry.Information.TypeKey, exception.what());
			}
			catch (...) {
				AYIN_CORE_ERROR("System '{}' failed during OnEnd", entry.Information.TypeKey);
			}
		}

		m_BegunSystems.clear();
		m_LifecycleState = LifecycleState::Idle;

	};


	void SystemSchedule::Shutdown(const SystemContext& systemContext) {

		if (IsActive())
			End(systemContext);

		ClearSystems();

	};


	bool SystemSchedule::BeginConstruction() {

		if (m_TopologySealed || !m_Systems.empty())
			return false;

		m_AttachSequence.clear();
		m_BeginPlan.clear();
		for (auto& phasePlan : m_PhasePlans)
			phasePlan.clear();

		return true;

	};


	bool SystemSchedule::BuildSystem(const SystemDefinition& definition) {

		if (m_TopologySealed) {
			AYIN_CORE_ERROR("SystemSchedule topology is already sealed");
			return false;
		}

		const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(definition.Type);
		if (descriptor == nullptr) {
			AYIN_CORE_ERROR("System '{}' is not registered", definition.Type);
			return false;
		}

		if (FindSystem(descriptor->RuntimeId) != m_Systems.end()) {
			AYIN_CORE_ERROR("System '{}' has already been added", descriptor->TypeKey);
			return false;
		}

		Scope<ISystem> instance = SystemRegistry::CreateSystemBy(descriptor->RuntimeId);
		if (!instance) {
			AYIN_CORE_ERROR("Failed to create system '{}'", descriptor->TypeKey);
			return false;
		}

		if (definition.Configuration.Json != "{}") {
			const auto result = SystemRegistry::DeserializeConfiguration(
				*instance, descriptor->RuntimeId, definition.Configuration.Json);
			if (!result) {
				AYIN_CORE_ERROR("Failed to deserialize system '{}': {}", descriptor->TypeKey, result.Error);
				return false;
			}
		}

		m_Systems.emplace_back(
			SystemInformation{.RuntimeId{descriptor->RuntimeId}, .TypeKey{descriptor->TypeKey}},
			SystemSpecification{.PhaseMask{definition.Specification.PhaseMask}, .ModeMask{definition.Specification.ModeMask}, .Order{definition.Specification.Order}},
			std::move(instance));


		return true;

	};


	bool SystemSchedule::FinishConstruction() {

		RebuildExecutionPlans();
		m_TopologySealed = true;
		return true;

	};


	bool SystemSchedule::AttachSystems() {

		if (!m_TopologySealed || IsActive()) {
			AYIN_CORE_WARN("SystemSchedule must be built and idle before Attach");
			return false;
		}

		if (m_Attached)
			return true;

		if (!m_AttachSequence.empty())
			DetachSystems();

		for (SystemIndex index = 0; index < m_Systems.size(); ++index) {
			SystemEntry& entry = m_Systems[index];

			try {
				entry.Instance->OnAttach();
				m_AttachSequence.emplace_back(index);
			}
			catch (const std::exception& exception) {
				AYIN_CORE_ERROR("System '{}' failed during OnAttach: {}", entry.Information.TypeKey, exception.what());
				DetachSystems();
				return false;
			}
			catch (...) {
				AYIN_CORE_ERROR("System '{}' failed during OnAttach", entry.Information.TypeKey);
				DetachSystems();
				return false;
			}
		}

		m_Attached = true;
		return true;

	};


	void SystemSchedule::ClearSystems() {

		if (IsActive()) {
			AYIN_CORE_WARN("SystemSchedule must End before clearing systems");
			return;
		}

		DetachSystems();
		m_Systems.clear();
		m_AttachSequence.clear();
		m_BeginPlan.clear();
		for (auto& phasePlan : m_PhasePlans)
			phasePlan.clear();
		m_BegunSystems.clear();
		m_TopologySealed = false;
		m_Attached = false;

	};


	void SystemSchedule::DetachSystems() {

		if (IsActive()) {
			AYIN_CORE_WARN("SystemSchedule must End before Detach");
			return;
		}

		if (!m_Attached && m_AttachSequence.empty())
			return;

		for (auto it = m_AttachSequence.rbegin(); it != m_AttachSequence.rend(); ++it) {
			if (*it >= m_Systems.size())
				continue;

			SystemEntry& entry = m_Systems[*it];
			if (!entry.Instance)
				continue;

			try {
				entry.Instance->OnDetach();
			}
			catch (const std::exception& exception) {
				AYIN_CORE_ERROR("System '{}' failed during OnDetach: {}", entry.Information.TypeKey, exception.what());
			}
			catch (...) {
				AYIN_CORE_ERROR("System '{}' failed during OnDetach", entry.Information.TypeKey);
			}
		}

		m_AttachSequence.clear();
		m_Attached = false;

	};


	std::vector<SystemSchedule::SystemIndex>& SystemSchedule::GetPhasePlan(const SystemPhase phase) {

		switch (phase) {
		case SystemPhase::PreUpdate: return m_PhasePlans[0];
		case SystemPhase::Update: return m_PhasePlans[1];
		case SystemPhase::PostUpdate: return m_PhasePlans[2];
		case SystemPhase::Presentation: return m_PhasePlans[3];
		default:
			AYIN_CORE_ASSERT(false, "Invalid SystemPhase for SystemSchedule");
			return m_PhasePlans[0];
		}

	};

	const std::vector<SystemSchedule::SystemIndex>& SystemSchedule::GetPhasePlan(const SystemPhase phase) const {

		return const_cast<SystemSchedule*>(this)->GetPhasePlan(phase);

	};


	void SystemSchedule::RebuildExecutionPlans() {

		m_BeginPlan.clear();
		for (auto& phasePlan : m_PhasePlans)
			phasePlan.clear();

		for (SystemIndex index = 0; index < m_Systems.size(); ++index) {
			const SystemSpecification& specification = m_Systems[index].Specification;
			m_BeginPlan.emplace_back(index);

			if (static_cast<bool>(specification.PhaseMask & SystemPhase::PreUpdate))
				GetPhasePlan(SystemPhase::PreUpdate).emplace_back(index);
			if (static_cast<bool>(specification.PhaseMask & SystemPhase::Update))
				GetPhasePlan(SystemPhase::Update).emplace_back(index);
			if (static_cast<bool>(specification.PhaseMask & SystemPhase::PostUpdate))
				GetPhasePlan(SystemPhase::PostUpdate).emplace_back(index);
			if (static_cast<bool>(specification.PhaseMask & SystemPhase::Presentation))
				GetPhasePlan(SystemPhase::Presentation).emplace_back(index);
		}

		const auto sortByOrder = [this](std::vector<SystemIndex>& plan) -> void {
			std::stable_sort(
				plan.begin(), plan.end(),
				[this](const SystemIndex left, const SystemIndex right) -> bool {
					return m_Systems[left].Specification.Order < m_Systems[right].Specification.Order;
				});
		};

		sortByOrder(m_BeginPlan);
		for (auto& phasePlan : m_PhasePlans)
			sortByOrder(phasePlan);

	};


	void SystemSchedule::MoveFrom(SystemSchedule&& other) noexcept {

		m_Systems = std::move(other.m_Systems);
		m_AttachSequence = std::move(other.m_AttachSequence);
		m_BeginPlan = std::move(other.m_BeginPlan);
		m_PhasePlans = std::move(other.m_PhasePlans);
		m_BegunSystems = std::move(other.m_BegunSystems);
		m_LifecycleState = other.m_LifecycleState;
		m_TopologySealed = other.m_TopologySealed;
		m_Attached = other.m_Attached;

		other.m_AttachSequence.clear();
		other.m_BeginPlan.clear();
		for (auto& phasePlan : other.m_PhasePlans)
			phasePlan.clear();
		other.m_BegunSystems.clear();
		other.m_LifecycleState = LifecycleState::Idle;
		other.m_TopologySealed = false;
		other.m_Attached = false;

	};


	std::vector<SystemEntry>::iterator SystemSchedule::FindSystem(const SystemID systemId) {

		return std::ranges::find_if(
			m_Systems,
			[systemId](const SystemEntry& entry) -> bool {
				return entry.Information.RuntimeId == systemId;
			});

	};

	std::vector<SystemEntry>::const_iterator SystemSchedule::FindSystem(const SystemID systemId) const {

		return std::ranges::find_if(
			m_Systems,
			[systemId](const SystemEntry& entry) -> bool {
				return entry.Information.RuntimeId == systemId;
			});

	};


	ISystem* SystemSchedule::FindSystemInstance(const SystemID systemId) {

		auto it = FindSystem(systemId);
		return it == m_Systems.end() ? nullptr : it->Instance.get();

	};

	const ISystem* SystemSchedule::FindSystemInstance(const SystemID systemId) const {

		auto it = FindSystem(systemId);
		return it == m_Systems.end() ? nullptr : it->Instance.get();

	};

	ISystem* SystemSchedule::FindSystemInstance(const std::string_view systemName) {

		auto it = std::ranges::find_if(
			m_Systems,
			[systemName](const SystemEntry& entry) -> bool {
				return entry.Information.TypeKey == systemName;
			});
		return it == m_Systems.end() ? nullptr : it->Instance.get();

	};

	const ISystem* SystemSchedule::FindSystemInstance(const std::string_view systemName) const {

		auto it = std::ranges::find_if(
			m_Systems,
			[systemName](const SystemEntry& entry) -> bool {
				return entry.Information.TypeKey == systemName;
			});
		return it == m_Systems.end() ? nullptr : it->Instance.get();

	};

	const SystemEntry* SystemSchedule::FindSystemEntry(const SystemID systemId) const {

		auto it = FindSystem(systemId);
		return it == m_Systems.end() ? nullptr : &*it;

	};

	const SystemEntry* SystemSchedule::FindSystemEntry(const std::string_view systemName) const {

		auto it = std::ranges::find_if(
			m_Systems,
			[systemName](const SystemEntry& entry) -> bool {
				return entry.Information.TypeKey == systemName;
			});
		return it == m_Systems.end() ? nullptr : &*it;

	};


};