#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Events/KeyEvent.h"
#include "Ayin/Events/MouseEvent.h"
#include "Ayin/Events/ApplicationEvent.h"

#include "Ayin/Core/Timestep.h"

#include "Ayin/Renderer/Camera.h"


namespace Ayin{

    class AYIN_API CameraController  : Camera{

    public:
        CameraController(const CameraProp& cameraProp);
        ~CameraController() = default;

        void OnUpdate(Timestep deltaTime);
        void OnEvent(Event& e);

		// 获取相机（主要是矩阵）
        inline const Camera& GetCamera() const { return *this; };
		// 获取相机参数
		inline void SetCameraProp(const CameraProp& cameraProp) { m_CameraProp = cameraProp; RecalculateProjectionMatrix(); };
        inline const CameraProp& GetCameraProp() const { return m_CameraProp; };
		inline const void SwitchCameraType(Camera::CameraType cameraType) { m_CameraProp.Type = cameraType; RecalculateProjectionMatrix(); }
		inline CameraType GetCameraType() const { return m_CameraProp.Type; };
		inline const void SetCameraFOV_H(float verticalFovAngle) { m_CameraProp.FOV = verticalFovAngle; RecalculateProjectionMatrix(); }
		inline float GetCameraFOV_H() const { return m_CameraProp.FOV; };

		// 相机位置设置
		inline void SetPosition(const glm::vec3& position) { m_CameraPosition = position; RecalculateViewMatrix(); };
		// 相机位置获取
		inline const glm::vec3& GetPosition() const { return m_CameraPosition; };

		// 相机姿态设置
		inline void SetRotation(const glm::vec3& rotation) { m_CameraRotation = rotation; RecalculateViewMatrix(); };
		// 相机姿态获取
		inline const glm::vec3& GetRotation() const { return m_CameraRotation; };



		inline const glm::mat4& GetRotationMatrix() const {
			glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_CameraRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_CameraRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_CameraRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			return pitch * yaw * roll;
		};



	private:

		/// <summary>
		/// 重新计算相机的视图矩阵以及VP矩阵(不包含投影矩阵)
		/// </summary>
		void RecalculateViewMatrix();

		void RecalculateProjectionMatrix();

    private:
        bool OnMouseScrolled(MouseScrolledEvent& e);
        bool OnMouseScrolled(Timestep deltaTime);
        bool OnMouseMidButtonPressed(MouseButtonPressedEvent& e);
        bool OnMouseMidButtonReleased(MouseButtonReleasedEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

        void OnAxisKeyPressed(Ayin::Timestep deltaTime);
        void OnMouseMoved(Ayin::Timestep deltaTime);



    private:

        CameraProp m_CameraProp;

        bool m_isRotate = false;                                                //是否开启旋转

        float m_ZoomLevel = 1.0f;                                               //缩放

        glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 1.0f };						// 位置
        glm::vec3 m_CameraRotation = { 0.0f, 0.0f, 0.0f };						// 角度

        float m_CameraTranslateSpeed = 1.0f, m_CameraRotationSpeed = 120.0f;

    };


}
