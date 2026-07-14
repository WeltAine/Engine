#pragma once

#include "Ayin/Core/Core.h"

#include "Ayin/Renderer/Camera.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp> // mat4x4

#include <glaze/glaze.hpp>


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
		inline const CameraProp& GetCameraProp() const { return m_CameraProp;};

		inline void SetCameraSize(int width, int height) { m_CameraProp.AspectRatio = (float)width / (float)height; RecalculateProjectionMatrix(); };
		inline void SetCameraFOVRadians(float angleRadians) { m_CameraProp.FOVRadians = angleRadians; RecalculateProjectionMatrix();};

		inline void SetCameraZoomLevel(float zoomLevel) { m_ZoomLevel = std::clamp(zoomLevel, 0.25f, 1.0f); };
		inline float GetCameraZoomLevel() const { return m_ZoomLevel; };


	private:

		/// <summary>
		/// 重新计算相机的视图矩阵以及VP矩阵(不包含投影矩阵)
		/// </summary>
		void RecalculateViewMatrix(const glm::vec3& position, const glm::vec3& rotation);

		void RecalculateProjectionMatrix();

	private:

		CameraProp m_CameraProp;

		float m_ZoomLevel = 1.0f;	//缩放


	private:

		// Glaze 序列化 CameraProp 时的额外处理
		inline bool OnCameraPropRead() {
			RecalculateProjectionMatrix();
			return true;
		}
		// Glaze 反序列化 CameraProp 时的额外处理
		inline bool OnCameraPropWrite() { return true; }

	public:
		struct glaze {
			using T = SceneCamera;
			static constexpr auto value = glz::object(
				"CameraProp", glz::manage<&T::m_CameraProp, &T::OnCameraPropRead, &T::OnCameraPropWrite>,
				//! glz::manage 的意思是：读写某个字段时，额外调用回调函数。
				//! 这里绑定到 m_CameraProp，所以 Glaze 读完 "CamreaProp" 后会调用 OnCameraPropRead()，进行 VP 重算。
				"ZoomLevel", &T::m_ZoomLevel
			);
		};
	};


}
