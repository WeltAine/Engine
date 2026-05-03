#include <AyinPch.h>

#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity

#include <glm/ext/matrix_clip_space.hpp> // perspective


#include "Ayin/Scene/SceneCamera.h"

namespace Ayin {
	

	SceneCamera::SceneCamera() {

		// 矩阵的结果是映射为一个[2, 2, 2]的大小（当然没有齐次化）
		switch (m_CameraProp.Type) {

			case(Camera::CameraType::Orthogonal): {

				float right = m_CameraProp.Height * m_CameraProp.AspectRatio * 0.5f;
				float top = m_CameraProp.Height * 0.5f;

				m_ProjectionMatrix = glm::ortho(-right, right, -top, top,
					m_CameraProp.NearPlaneDistance, m_CameraProp.FarPlaneDistance);

				break;
			};

			case(Camera::CameraType::Perspective): {

				m_ProjectionMatrix = glm::perspective(
					glm::radians(m_CameraProp.FOV),
					m_CameraProp.AspectRatio,
					m_CameraProp.NearPlaneDistance, m_CameraProp.FarPlaneDistance);

				break;
			};

		}

		m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;

	}


	SceneCamera::SceneCamera(const CameraProp& cameraProp) 
		:m_CameraProp{ cameraProp }
	{
		// 矩阵的结果是映射为一个[2, 2, 2]的大小（当然没有齐次化）
		switch (cameraProp.Type) {

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

	};


	void SceneCamera::SetProjection(const CameraProp& cameraProp) {
		AYIN_PROFILE_FUNCTION();

		m_CameraProp = cameraProp;

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

	};

	void SceneCamera::SetCameraSize(int width, int height) {

		m_CameraProp.AspectRatio = (float)width / (float)height; 
		SetProjection(m_CameraProp); 

	};

	void SceneCamera::SetCameraFOV(float angle) { 

		m_CameraProp.FOV = angle; 
		SetProjection(m_CameraProp);

	};


	void SceneCamera::RecalculateViewMatrix(const glm::vec3& position, const glm::vec3& rotation) {
		AYIN_PROFILE_FUNCTION();

		glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), position) * pitch * yaw * roll;

		m_ViewMatrix = glm::inverse(transform);

		m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;
	};

}