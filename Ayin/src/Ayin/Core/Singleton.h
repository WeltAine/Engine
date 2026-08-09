#pragma once

#include <Ayin/Core/Core.h>

namespace Ayin {

	//Todo: 多线程支持
	template<typename T>
	class Singleton {

	public:

		inline static T& Instance() { static T s_Instance{}; return s_Instance; }

		Singleton(const Singleton&) = delete;
		Singleton& operator=(const Singleton&) = delete;
		Singleton(Singleton&&) = delete;
		Singleton& operator=(Singleton&&) = delete;

	protected:

		Singleton() = default;
		~Singleton() = default;

	};

}
