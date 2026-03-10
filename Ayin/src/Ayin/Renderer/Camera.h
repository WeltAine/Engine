#pragma once

#include "Ayin/Core.h"
#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp> // mat4x4

#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity


namespace Ayin {

	/// <summary>
	/// 投影矩阵参数
	/// </summary>
	struct AYIN_API PerspectiveProp {

		float NearPlaneDistance, FarPlaneDistance;

		float FOV;

		float AspectRatio;

	};

	/// <summary>
	/// 正交矩阵参数
	/// </summary>
	struct AYIN_API OrthogonalProp {

		float NearPlaneDistance, FarPlaneDistance;

		float Height;

		float AspectRatio;

	};

	/// <summary>
	/// 相机参数联合体
	/// </summary>
	/// 联合体的初始化???
	union CameraProp
	{
		PerspectiveProp perspectiveProp;

		OrthogonalProp orthogonalProp;
	};

	class AYIN_API Camera {

	public:

		/// <summary>
		/// 相机类型枚举
		/// </summary>
		enum class CameraType
		{
			None = 0,
			Perspective = 1,	// 透视
			Orthogonal = 2		// 正交
		};


	public:
		Camera(CameraType cameraType, const CameraProp& cameraProp);

		inline void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); };
		inline const glm::vec3& GetPosition() const { return m_Position; };

		inline void SetRotation(const glm::vec3& rotation) { m_Rotation = rotation; RecalculateViewMatrix(); };
		inline const glm::vec3& GetRotation() const { return m_Rotation; };

		inline CameraType GetCameraType() const { return m_CameraType; };

		inline const CameraProp& GetCameraProp() const { return m_CameraProp; };

		inline const glm::mat4& GetProjecttionViewMatrix() const { return m_ProjecttionViewMatrix; };

		inline const glm::mat4& GetRotationMatrix() const {
			glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			return pitch * yaw * roll;

		};

	private:

		/// <summary>
		/// 重新计算相机的视图矩阵以及VP矩阵(不包含投影矩阵)
		/// </summary>
		void RecalculateViewMatrix();


	private:
		glm::vec3 m_Position = {0.0f, 0.0f, 1.0f};						// 位置
		glm::vec3 m_Rotation = {0.0f, 0.0f, 0.0f};						// 角度

		CameraType m_CameraType;										// 相机类型

		CameraProp m_CameraProp;										// 相机参数

		glm::mat4 m_ViewMatrix = glm::identity<glm::mat4>();			// 视图矩阵
		glm::mat4 m_ProjectionMatrix = glm::identity<glm::mat4>();		// 投影矩阵
		glm::mat4 m_ProjecttionViewMatrix = glm::identity<glm::mat4>();	// VP矩阵

	};




}
