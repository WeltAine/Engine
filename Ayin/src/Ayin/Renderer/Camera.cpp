#include <AyinPch.h>

#include "Ayin/Renderer/Camera.h"

#include <glm/ext/vector_float2.hpp> // vec2
#include <glm/ext/vector_float3.hpp> // vec3
#include <glm/ext/matrix_float4x4.hpp> // mat4x4
#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity
#include <glm/ext/matrix_clip_space.hpp> // perspective

namespace Ayin {


	Camera::Camera(const CameraProp& cameraProp)
		:m_CameraType{ cameraProp.Type }
	{

		glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), m_Position) * pitch * yaw * roll;

		m_ViewMatrix = glm::inverse(transform);

		// 矩阵的结果是映射为一个[2, 2, 2]的大小（当然没有齐次化）
		switch (m_CameraType) {

			case(Camera::CameraType::Orthogonal): {

				float right = cameraProp.Height * cameraProp.AspectRatio * 0.5f;
				float top = cameraProp.Height * 0.5f;

				m_ProjectionMatrix = glm::ortho(-right, right, -top, top, 
					cameraProp.NearPlaneDistance, cameraProp.FarPlaneDistance);

				break;
			};

			case(Camera::CameraType::Perspective): {

				m_ProjectionMatrix = glm::perspective(glm::radians(cameraProp.FOV), 
					cameraProp.AspectRatio, 
					cameraProp.NearPlaneDistance, 
					cameraProp.FarPlaneDistance);

				break;
			};
		
		}

		m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;


	}


	void Camera::SetProjection(const CameraProp& cameraProp){

		AYIN_PROFILE_FUNCTION();

		m_CameraType = cameraProp.Type;

		switch (cameraProp.Type) {

			case(Camera::CameraType::Orthogonal): {

				float right = cameraProp.Height * cameraProp.AspectRatio * 0.5f;
				float top = cameraProp.Height * 0.5f;

				m_ProjectionMatrix = glm::ortho(
					-right, right, -top, top, 
					cameraProp.NearPlaneDistance, 
					cameraProp.FarPlaneDistance);

				break;
			};

			case(Camera::CameraType::Perspective): {

				m_ProjectionMatrix = glm::perspective(
					glm::radians(cameraProp.FOV), 
					cameraProp.AspectRatio, 
					cameraProp.NearPlaneDistance, 
					cameraProp.FarPlaneDistance);

				break;
			};
		
		}

		m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;


	}


	void Camera::RecalculateViewMatrix()
	{
		AYIN_PROFILE_FUNCTION();

		glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), m_Position) * pitch * yaw * roll;

		m_ViewMatrix = glm::inverse(transform);

		m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	 
}