#pragma once

#include "Ayin/Core/Core.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp> // mat4x4
#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity


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

	};


	/// <summary>
	/// 相机参数结构体
	/// <para> Type(透视) </para>
	/// <para> FOV(60),	Height(2) </para>
	/// <para> AspectRatio(16:9) </para>
	/// <para> NearPlaneDistance(0.1), FarPlaneDistance(100) </para>
	/// </summary>
	struct AYIN_API CameraProp
	{
		Camera::CameraType Type = Camera::CameraType::Perspective;

		float FOV = 60, Height = 2;//高尽可能不变，该参数为正交相机使用，一般只有缩放可以改变它，大多数情况不应该直接控制

		float AspectRatio = 16.0f / 9.0f;

		float NearPlaneDistance = 0.1f, FarPlaneDistance = 100.0f;
	};


}
