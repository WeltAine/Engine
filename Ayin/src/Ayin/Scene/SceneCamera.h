#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Renderer/Camera.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp> // mat4x4


namespace Ayin {


	class AYIN_API SceneCamera : public Camera {


	public:

		SceneCamera();
		SceneCamera(const CameraProp& cameraProp);

		~SceneCamera() = default;

		inline Camera::CameraType GetCameraType() const { return m_CameraProp.Type; };

		inline void SetViewMatrix(const glm::vec3& position, const glm::vec3& rotation) { RecalculateViewMatrix(position, rotation); };
		inline void SetCameraMode(Camera::CameraType mode) { m_CameraProp.Type = mode; RecalculateProjectionMatrix(); };
		inline void SetProjection(const CameraProp& cameraProp) { m_CameraProp = cameraProp; RecalculateProjectionMatrix(); };

		inline void SetCameraSize(int width, int height) { m_CameraProp.AspectRatio = (float)width / (float)height; RecalculateProjectionMatrix(); };
		inline void SetCameraFOV(float angle) { m_CameraProp.FOV = angle; RecalculateProjectionMatrix();};

		inline void SetCameraZoomLevel(float zoomLevel) { m_ZoomLevel = std::clamp(zoomLevel, 0.25f, 1.0f); };
		inline float GetCameraZoomLevel() const { return m_ZoomLevel; };

		inline const CameraProp GetCameraProp() const { return m_CameraProp;};

	private:

		/// <summary>
		/// 重新计算相机的视图矩阵以及VP矩阵(不包含投影矩阵)
		/// </summary>
		void RecalculateViewMatrix(const glm::vec3& position, const glm::vec3& rotation);

		void RecalculateProjectionMatrix();

	private:

		CameraProp m_CameraProp;

		float m_ZoomLevel = 1.0f;	//缩放
	};


}
