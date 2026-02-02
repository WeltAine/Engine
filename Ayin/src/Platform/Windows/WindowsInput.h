#pragma once

#include "Ayin/Input.h"

namespace Ayin {

	class WindowsInput : public Input
	{
	protected:
		// 通过 Input 继承
		virtual bool IsKeyPressedImpl(int keyCode) const override;

		virtual bool GetMouseButtonImpl(int button) const override;
		virtual std::pair<float, float> GetMousePositionImpl() const override;
		virtual float GetMouseXImpl() const override;
		virtual float GetMouseYImpl() const override;

	};

}

