#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Renderer/Camera.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp> // mat4x4


namespace Ayin {


	class AYIN_API SceneCamera : public Camera_ {


	public:

		SceneCamera();
		SceneCamera(const CameraProp& cameraProp);

		~SceneCamera() = default;

		inline Camera::CameraType GetCameraType() const { return m_CameraProp.Type; };

		inline void SetViewMatrix(const glm::vec3& position, const glm::vec3& rotation) { RecalculateViewMatrix(position, rotation); };
		void SetProjection(const CameraProp& cameraProp);

		void SetCameraSize(int width, int height);
		void SetCameraFOV(float angle);

		inline const CameraProp GetCameraProp() const { return m_CameraProp; };

	private:

		/// <summary>
		/// 重新计算相机的视图矩阵以及VP矩阵(不包含投影矩阵)
		/// </summary>
		void RecalculateViewMatrix(const glm::vec3& position, const glm::vec3& rotation);

	private:

		CameraProp m_CameraProp;

	};


}