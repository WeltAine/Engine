#include <AyinPch.h>

#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity

#include <glm/ext/matrix_clip_space.hpp> // perspective
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>


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
					m_CameraProp.FOVRadians,
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

				m_ProjectionMatrix = glm::perspective(cameraProp.FOVRadians,
					cameraProp.AspectRatio,
					cameraProp.NearPlaneDistance,
					cameraProp.FarPlaneDistance);

				break;
			};

		}

		m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;

	};



	void SceneCamera::RecalculateViewMatrix(const glm::vec3& position, const glm::vec3& rotation) {
		AYIN_PROFILE_FUNCTION();

		glm::mat4 rotationMatrix = glm::eulerAngleXYZ(rotation.x, rotation.y, rotation.z);

		glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), position) * rotationMatrix;

		m_ViewMatrix = glm::inverse(transform);

		m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;
	};


	void SceneCamera::RecalculateProjectionMatrix() {
	
		AYIN_PROFILE_FUNCTION();

		switch (m_CameraProp.Type) {

			case(Camera::CameraType::Orthogonal): {

				float right = m_CameraProp.Height * m_CameraProp.AspectRatio * 0.5f;
				float top = m_CameraProp.Height * 0.5f;

				m_ProjectionMatrix = glm::ortho(
					-right, right, -top, top,
					m_CameraProp.NearPlaneDistance,
					m_CameraProp.FarPlaneDistance);

				break;
			};

			case(Camera::CameraType::Perspective): {

				m_ProjectionMatrix = glm::perspective(
					m_CameraProp.FOVRadians,
					m_CameraProp.AspectRatio,
					m_CameraProp.NearPlaneDistance,
					m_CameraProp.FarPlaneDistance);

				break;
			};

		}

		m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;

	
	};


}
