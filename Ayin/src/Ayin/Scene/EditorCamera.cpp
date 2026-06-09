#include <AyinPch.h>

#include "Ayin/Scene/EditorCamera.h"
#include "Ayin/Math/Math.h"

#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity
#include <glm/ext/matrix_clip_space.hpp> // perspective

namespace Ayin {


	EditorCamera::EditorCamera() {
	
		m_Orientation = glm::quatLookAt(glm::normalize(-m_Position), glm::vec3{ 0.0f, 1.0f, 0.0f });

		UpdateViewMatrix();
		UpdateProjectionMatrix();

	};

	
	EditorCamera::EditorCamera(float height, float aspect, float fov, float nearPlane, float farPlane)
		:BaseHeight(height), BaseFOV(fov), minFOVZoom(BaseFOV / 150.0f)
	{
	
		m_Orientation = glm::quatLookAt(glm::normalize(-m_Position), glm::vec3{ 0.0f, 1.0f, 0.0f });

		m_CameraProp.Height = BaseHeight;
		m_CameraProp.FOV = BaseFOV;

		m_CameraProp.AspectRatio = aspect;

		m_CameraProp.NearPlaneDistance = nearPlane;
		m_CameraProp.FarPlaneDistance = farPlane;

		UpdateViewMatrix();
		UpdateProjectionMatrix();

	};


	void EditorCamera::OnUpdate(Timestep deltaTime) {
	

		glm::vec3 deltaPosition{ 0.0f };
		
		if (Input::IsKeyPressed(KeyCode::A)) deltaPosition -= glm::vec3{1.0f, 0.0f, 0.0f};
		if (Input::IsKeyPressed(KeyCode::D)) deltaPosition += glm::vec3{1.0f, 0.0f, 0.0f};
		//GLM提供的相机默认是朝着负Z方向看的
		if (Input::IsKeyPressed(KeyCode::W)) deltaPosition -= glm::vec3{0.0f, 0.0f, 1.0f};
		if (Input::IsKeyPressed(KeyCode::S)) deltaPosition += glm::vec3{0.0f, 0.0f, 1.0f};

		if (Input::IsKeyPressed(KeyCode::Q)) deltaPosition += glm::vec3{ 0.0f, 1.0f, 0.0f };
		if (Input::IsKeyPressed(KeyCode::E)) deltaPosition -= glm::vec3{ 0.0f, 1.0f, 0.0f };


		Move(deltaPosition, deltaTime);

		glm::vec2 mousePosition = Input::GetMousePosition();

		if (Input::IsMouseButtonPressed(MouseCode::Mouse_Button_2)) {

			if (!m_IsRotating) {
				m_LastMousePosition = mousePosition;
				m_IsRotating = true;
			}

			Rotate(mousePosition - m_LastMousePosition);
			m_LastMousePosition = mousePosition;

		}
		else {
			m_IsRotating = false;
		}

		Zoom(Input::GetScrollYoffset() * m_ZoomSpeed);
	
	};


	void EditorCamera::SetEditorCameraMode(Camera::CameraType cameraType) { 
		
		m_CameraProp.Type = cameraType; 

		switch (m_CameraProp.Type) {
		
		case(Camera::CameraType::Orthogonal): {

			m_ZoomLevel = BaseHeight / m_CameraProp.Height;
			break;

		};
		case(Camera::CameraType::Perspective): {
		
			m_ZoomLevel = BaseFOV / m_CameraProp.FOV;
			break;

		}

		};
	
		UpdateProjectionMatrix();

	};


	void EditorCamera::Move(const glm::vec3& deltaPosition, Timestep deltaTime) {

		if (glm::length(deltaPosition) == 0.0f)
			return;

		glm::vec3 _deltaPosition = glm::normalize(deltaPosition);

		glm::vec3 translate = glm::toMat4(m_Orientation) * glm::vec4{ _deltaPosition, 0.0f } * float(deltaTime);

		m_Position += translate;

		UpdateViewMatrix();

	};
	void EditorCamera::Rotate(const glm::vec2& deltaRotation) {

		if (glm::length(deltaRotation) == 0.0f)
			return;

		glm::vec3 localRight = m_Orientation * glm::vec3{ 1.0f, 0.0f, 0.0f };

		glm::quat pitch = glm::angleAxis(glm::radians(-deltaRotation.y * m_RotateSpeed), localRight);		//绕局部X轴旋转
		glm::quat yaw = glm::angleAxis(glm::radians(-deltaRotation.x * m_RotateSpeed), glm::vec3{ 0.0f, 1.0f, 0.0f });		//绕全局Y轴旋转

		m_Orientation = glm::normalize(yaw * pitch * m_Orientation);
		//所以这并没有避开万向死锁，不过也没关系

		UpdateViewMatrix();

	};
	void EditorCamera::Zoom(float deltaZoom) {
		

		m_ZoomLevel += deltaZoom;
		m_ZoomLevel = std::clamp(m_ZoomLevel, 0.1f, 20.0f);

		if (m_CameraProp.Type == Camera::CameraType::Orthogonal) {
			
			m_CameraProp.Height = BaseHeight / m_ZoomLevel;
		
		}
		else {

			m_ZoomLevel = std::max(m_ZoomLevel, minFOVZoom);//确保放大不要超过150°
			
			m_CameraProp.FOV = std::clamp(BaseFOV / m_ZoomLevel, 1.0f, 150.0f);
		
		}

		UpdateProjectionMatrix();
	
	};

	void EditorCamera::UpdateViewMatrix() {
		
		glm::mat4 transform = glm::translate(glm::mat4{1.0f}, m_Position) * glm::toMat4(m_Orientation);

		m_ViewMatrix = glm::inverse(transform);

		m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;
	
	};
	void EditorCamera::UpdateProjectionMatrix() {
	
		float right = m_CameraProp.Height * m_CameraProp.AspectRatio * 0.5f;
		float top = m_CameraProp.Height * 0.5f;
		

		switch (m_CameraProp.Type) {
			
			case(Camera::CameraType::Orthogonal): {

				m_ProjectionMatrix = glm::ortho(-right, right, -top, top, m_CameraProp.NearPlaneDistance, m_CameraProp.FarPlaneDistance);
				break;

			};
			case(Camera::CameraType::Perspective): {

				m_ProjectionMatrix = glm::perspective(glm::radians(m_CameraProp.FOV), m_CameraProp.AspectRatio, m_CameraProp.NearPlaneDistance, m_CameraProp.FarPlaneDistance);
				break;

			};		
		
		}


		m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;

		return;

	};


}
