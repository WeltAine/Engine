#include <AyinPch.h>

#include "Camera.h"

#include <glm/ext/vector_float2.hpp> // vec2
#include <glm/ext/vector_float3.hpp> // vec3
#include <glm/ext/matrix_float4x4.hpp> // mat4x4
#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity
#include <glm/ext/matrix_clip_space.hpp> // perspective

namespace Ayin {


	Camera::Camera(CameraType cameraType, const CameraProp& cameraProp)
		:m_CameraType{ cameraType }, m_CameraProp{ cameraProp }
	{

		glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), m_Position) * pitch * yaw * roll;

		m_ViewMatrix = glm::inverse(transform);

		// 矩阵的结果是映射为一个[2, 2, 2]的大小（当然没有齐次化）
		switch (cameraType) {

			case(Camera::CameraType::Orthogonal): {

				float right = cameraProp.orthogonalProp.Height * cameraProp.orthogonalProp.AspectRatio * 0.5f;
				float top = cameraProp.orthogonalProp.Height * 0.5f;

				m_ProjectionMatrix = glm::ortho(-right, right, -top, top, 
					cameraProp.orthogonalProp.NearPlaneDistance, cameraProp.orthogonalProp.FarPlaneDistance);

				break;
			};

			case(Camera::CameraType::Perspective): {

				m_ProjectionMatrix = glm::perspective(glm::radians(cameraProp.perspectiveProp.FOV), 
					cameraProp.perspectiveProp.AspectRatio, 
					cameraProp.perspectiveProp.NearPlaneDistance, 
					cameraProp.perspectiveProp.FarPlaneDistance);

				break;
			};
		
		}

		m_ProjecttionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;


	}

	void Camera::RecalculateViewMatrix()
	{
		glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), m_Position) * pitch * yaw * roll;

		m_ViewMatrix = glm::inverse(transform);

		m_ProjecttionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}


}