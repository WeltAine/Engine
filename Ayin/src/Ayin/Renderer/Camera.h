#pragma once

#include "Ayin/Core/Core.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp> // mat4x4
#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity

#include <glaze/glaze.hpp>


namespace Ayin {
	
	struct CameraProp;



	class AYIN_API Camera {

	public:
		/// <summary>
		/// 相机类型枚举
		/// </summary>
		enum class CameraType : uint8_t
		{
			None = 0,
			Perspective = 1,	// 透视
			Orthogonal = 2		// 正交
		};

	public:

		Camera() = default;
		~Camera() = default;

		inline const glm::mat4& GetProjectionViewMatrix() const { return m_ProjectionViewMatrix; };
		inline const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; };
		inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; };

	protected:

		glm::mat4 m_ViewMatrix = glm::identity<glm::mat4>();			// 视图矩阵
		glm::mat4 m_ProjectionMatrix = glm::identity<glm::mat4>();		// 投影矩阵
		glm::mat4 m_ProjectionViewMatrix = glm::identity<glm::mat4>();	// VP矩阵

	private:

		// Glaze 反序列化 m_ProjectionMatrix 时的额外处理
		inline bool ReadProjectionMatrixForGlaze() { m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix; return true; };
		// Glaze 序列化 m_ProjectionMatrix 时的额外处理
		bool WriteProjectionMatrixForGlaze() { return true; };
		//! glz::manage 要求两个额外处理函数要返回是否处理成功的信息


	public:

		struct glaze {
			using T = Camera;
			static constexpr auto value = glz::object(
				"ViewMatrix", &T::m_ViewMatrix,
				"ProjectionMatrix", glz::manage<&T::m_ProjectionMatrix, 
				&T::ReadProjectionMatrixForGlaze, &T::WriteProjectionMatrixForGlaze>
				//! glz::manage 的意思是：读写某个字段时，额外调用回调函数。
				//! 这里绑定到 m_ProjectionMatrix，所以 Glaze 读完 "ProjectionMatrix" 后会调用 ReadProjectionMatrixForGlaze()，进行 VP 重算。
			);
		};

	};


	/// <summary>
	/// 相机参数结构体
	/// <para> Type(透视) </para>
	/// <para> FOVRadians(60 degrees),	Height(2) </para>
	/// <para> AspectRatio(16:9) </para>
	/// <para> NearPlaneDistance(0.1), FarPlaneDistance(100) </para>
	/// </summary>
	struct AYIN_API CameraProp
	{
		Camera::CameraType Type = Camera::CameraType::Perspective;

		float FOVRadians = glm::radians(60.0f), Height = 2;//高尽可能不变，该参数为正交相机使用，一般只有缩放可以改变它，大多数情况不应该直接控制（除非你的实现中准备了自己的控制参数）

		float AspectRatio = 16.0f / 9.0f;

		float NearPlaneDistance = 0.1f, FarPlaneDistance = 100.0f;

		struct glaze {
			using T = CameraProp;
			static constexpr auto value = glz::object(
				"Type", &T::Type,
				"FOVRadians", &T::FOVRadians,
				"Height", &T::Height,
				"AspectRatio", &T::AspectRatio,
				"NearPlane", &T::NearPlaneDistance,
				"FarPlane", &T::FarPlaneDistance
			);
		};
	};


}

template<>
struct glz::meta<Ayin::Camera::CameraType> {

	using enum Ayin::Camera::CameraType;

	static constexpr auto value = glz::enumerate(
		None,
		Perspective,
		Orthogonal
	);

};
