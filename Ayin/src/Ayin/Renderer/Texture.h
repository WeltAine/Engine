#pragma once

#include "Ayin/Core.h"

#include <string>

namespace Ayin {

	class AYIN_API Texture {

	public:
		virtual ~Texture() = default;

		virtual int GetWidth() const = 0;
		virtual int GetHeight() const = 0;
		virtual int GetComponents() const = 0;

		virtual void Bind(int slot) const = 0;
		virtual void UnBind() const = 0;
	};

	class AYIN_API Texture2D : public Texture {

	public:

		static Ref<Texture2D> Create(const std::string& path);

	};
}