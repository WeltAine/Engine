#pragma once

#include "Ayin/Core/Core.h"
#include "Ayin/Core/Timestep.h"
#include "Ayin/Renderer/Camera.h"
#include "Ayin/Events/Event.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>


namespace Ayin {

	class AYIN_API EditorCamera : public Camera {

	public:

		EditorCamera();
		EditorCamera(float height, float aspect, float fov, float nearPlane, float farPlane);

		~EditorCamera() = default;


		void OnUpdate(Timestep deltaTime);
		inline void OnEvent(Event& event) {};

		void SetEditorCameraMode(Camera::CameraType cameraType);

		inline void SetCameraAspect(float aspectRatio) { m_CameraProp.AspectRatio = aspectRatio; UpdateProjectionMatrix(); };

		inline CameraProp GetEditorCameraProp() const { return m_CameraProp; };

		inline Camera::CameraType GetCameraType() const { return m_CameraProp.Type; };

	private:

		void Move(const glm::vec3& deltaPosition, Timestep deltaTime);
		void Rotate(const glm::vec2& deltaRotation);
		void Zoom(float deltaZoom);

		void UpdateViewMatrix();
		void UpdateProjectionMatrix();

	private:

		const float BaseHeight = 2.0f;	//基础高度（不会变化）
		const float BaseFOV = 60.0f;	//基础FOV（不会变化）(最大150)
		const float minFOVZoom = 0.4f;

		float m_ZoomLevel = 1.0f;

		CameraProp m_CameraProp;

		glm::vec3 m_Position{ 3.0f, 3.0f, 10.0f };
		glm::quat m_Orientation{ 1.0f, 0.0f, 0.0f, 0.0f };	//没有任何旋转，glm::quat( w, x, y, z )，四元数公式：q = w + xi + yj + zk；w：实数部分（标量）→ 代表旋转角度，x, y, z：虚数部分（向量）→ 代表旋转轴

		float m_MoveSpeed = 5.0f;
		float m_RotateSpeed = 0.15f;
		float m_ZoomSpeed = 0.2f;

		bool m_IsRotating = false;
		glm::vec2 m_LastMousePosition{ 0.0f };

	};


}
