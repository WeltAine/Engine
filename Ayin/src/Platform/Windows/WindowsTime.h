#pragma once

#include "Ayin/Core/Time.h"

namespace Ayin {


	class WindowsTime : public Time{

	public:

		WindowsTime() = default;

		~WindowsTime() = default;


	private:

		inline virtual Timestep GetRunTimeImpl() const override { return m_RunTime; };

		inline virtual Timestep GetFrameInternalImpl() const override { return m_FrameInterval; };

		virtual void OnUpdateImpl() override;

	private:

		Timestep m_RunTime;
		Timestep m_FrameInterval;


	};


}