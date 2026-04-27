#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Core/Timestep.h"

#include "Ayin/Core/Log.h"

#include <GLFW/glfw3.h>


namespace Ayin {

	class AYIN_API Time {

		friend class Application;

	protected:

		Time() = default;

	public:

		Time(const Time& other) = delete;
		Time& operator = (const Time& other) = delete;

		virtual ~Time() = default;


	private:

		virtual Timestep GetRunTimeImpl() const = 0;

		virtual Timestep GetFrameInternalImpl() const = 0;

		virtual void OnUpdateImpl() = 0;


	public:

		inline static Timestep GetRunTime() { return s_Instance->GetRunTimeImpl(); }

		inline static Timestep GetFrameInterval() { return s_Instance->GetFrameInternalImpl(); }


	private:
		
		static Scope<Time> Create();

		inline static void OnUpdate() { s_Instance->OnUpdateImpl(); };

	private:

		static Scope<Time> s_Instance;

	};


}