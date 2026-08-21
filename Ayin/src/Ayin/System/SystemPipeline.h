#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/System/SystemTypes.h"
#include "Ayin/System/SystemRegistry.h"

#include <glaze/glaze.hpp>

#include <concepts>
#include <set>
#include <string_view>
#include <vector>


namespace Ayin {



	struct SystemJson;
	class SystemSchedule;

	struct AYIN_API SystemRegistration {

		SystemInformation Information;
		SystemSpecification Specification;

		::glz::raw_json SystemData = NullSystemData;

		static constexpr const char* NullSystemData = "{}";


		operator SystemJson() const;


		struct SystemRegistrationCompareByOrder {

			//? 比较器为什么一定是 const
			//! std::set 把比较器当作容器规则，而不是可变状态。它会通过一个 const Compare 对象调用： operator()
			//! 末尾的 const 表示比较过程不修改比较器自身。否则标准库无法在 const 上下文中安全使用它。
			inline bool operator() (const SystemRegistration& leftRegistration, const SystemRegistration& rightRegistration) const {

				if (leftRegistration.Information.RuntimeId != rightRegistration.Information.RuntimeId && leftRegistration.Specification.Order != rightRegistration.Specification.Order)
					return leftRegistration.Specification.Order < rightRegistration.Specification.Order;

				return leftRegistration.Information.RuntimeId < rightRegistration.Information.RuntimeId;

			};

		};


	};

	
	class AYIN_API SystemPipeline {

		friend class SystemScheduleSerializer;

	public:

		class AYIN_API Builder {
		public:
			using RegistrationSet = std::set<SystemRegistration, SystemRegistration::SystemRegistrationCompareByOrder>;

		private:
			RegistrationSet m_Registrations;
			int m_NextOrder = 0;

		public:

			Builder() = default;
			Builder(const SystemPipeline& pipeline);//基于已有管线构造 Builder

			SystemPipeline Build();


			Builder& AddSystem(const SystemRegistration& systemRegistration);

			template<typename System>
				requires std::derived_from<System, ISystem>&& std::default_initializable<System>
			Builder& AddSystem(const std::vector<SystemPhase>& phases, const std::vector<SceneMode>& modes);

			template<typename System>
				requires std::derived_from<System, ISystem>&& std::default_initializable<System>
			Builder& AddSystem(const std::vector<SystemPhase>& phases, const std::vector<SceneMode>& modes, int order);


			Builder& RemoveSystem(const SystemID systemId);
			Builder& RemoveSystem(const std::string_view systemName);
			Builder& RemoveSystemPhase(SystemID systemId, SystemPhase phase);


			Builder& SetSystemSpecification(SystemID systemId, const SystemSpecification& specification);


			inline const RegistrationSet& GetRegistrations() const { return m_Registrations; };


			bool ContainSystem(SystemID systemId) const;
			bool ContainSystem(const std::string_view systemName) const;

		private:

			RegistrationSet::iterator FindSystem(SystemID systemId);
			RegistrationSet::iterator FindSystem(const std::string_view systemName);
			RegistrationSet::const_iterator FindSystem(SystemID systemId) const;
			RegistrationSet::const_iterator FindSystem(const std::string_view systemName) const;

		};

	private:

		std::set<SystemRegistration, SystemRegistration::SystemRegistrationCompareByOrder> m_Registrations;

	public:

		void Build(SystemSchedule& schedule) const;
		SystemSchedule CreateSchedule() const;

	};



	template<typename System>
		requires std::derived_from<System, ISystem>&& std::default_initializable<System>
	inline SystemPipeline::Builder& SystemPipeline::Builder::AddSystem(const std::vector<SystemPhase>& phases, const std::vector<SceneMode>& modes) {
		//! 不能再使用 std::initializer_list，因为其底层记录的是指针，而非具体元素，我们会延迟使用，即使是对 std::initializer_list 进行值拷贝也没法复制到值

		return AddSystem<System>(phases, modes, m_NextOrder);

	};

	template<typename System>
		requires std::derived_from<System, ISystem>&& std::default_initializable<System>
	inline SystemPipeline::Builder& SystemPipeline::Builder::AddSystem(const std::vector<SystemPhase>& phases, const std::vector<SceneMode>& modes, int order) {

		SystemPhase phaseMask = SystemPhase::None;
		for (const SystemPhase phase : phases) {
			phaseMask |= phase;
		}

		SceneMode modeMask = SceneMode::None;
		for (const SceneMode mode : modes) {
			modeMask |= mode;
		}

		const SystemDescriptor* descriptor = SystemRegistry::GetSystemDescriptor(GetSystemID<System>());
		if (descriptor == nullptr) {
			AYIN_CORE_ERROR("System RuntimeId is not registered");
			return *this;
		}

		SystemRegistration systemRegistration{
			// Name 在 SystemDefinition 出现前暂存 TypeKey，不能再保存编译器相关的 RTTI 名称。
			.Information{.RuntimeId{descriptor->RuntimeId}, .Name{descriptor->TypeKey}},
			.Specification{.PhaseMask{phaseMask}, .ModeMask{modeMask}, .Order{order}}
		};

		AddSystem(systemRegistration);

		return *this;

	};


};
