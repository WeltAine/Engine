#include <AyinPch.h>

#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity

#include <glm/ext/matrix_clip_space.hpp> // perspective


#include "Ayin/CameraController.h"
#include "Ayin/Core/KeyCodes.h"
#include "Ayin/Core/MouseButtonCodes.h"
#include "Ayin/Core/Input.h"


namespace Ayin{


		CameraController::CameraController(const CameraProp& cameraProp)
			: m_CameraProp{cameraProp}
		{
			AYIN_ERROR("Now FOV: {0}", m_CameraProp.FOV);

			// 设置相机状态
			RecalculateViewMatrix();
			RecalculateProjectionMatrix();
		}

		void CameraController::OnUpdate(Timestep deltaTime){

			AYIN_PROFILE_FUNCTION();

			OnAxisKeyPressed(deltaTime);
			OnMouseMoved(deltaTime);
			OnMouseScrolled(deltaTime);
		}


		void CameraController::OnEvent(Event& e){

			AYIN_PROFILE_FUNCTION();

			EventDispatcher dispatcher{ e };

			dispatcher.Dispatch<MouseScrolledEvent>(AYIN_BIND_EVENT_FUN_OVERRIDE( bool(CameraController::*)(MouseScrolledEvent&), CameraController::OnMouseScrolled));
			dispatcher.Dispatch<MouseButtonPressedEvent>(AYIN_BIND_EVENT_FUN(CameraController::OnMouseMidButtonPressed));
			dispatcher.Dispatch<MouseButtonReleasedEvent>(AYIN_BIND_EVENT_FUN(CameraController::OnMouseMidButtonReleased));
			dispatcher.Dispatch<WindowResizeEvent>(AYIN_BIND_EVENT_FUN(CameraController::OnWindowResize));

		}



		void CameraController::RecalculateViewMatrix() {
			AYIN_PROFILE_FUNCTION();

			glm::mat4 pitch = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_CameraRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 yaw = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_CameraRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 roll = glm::rotate(glm::identity<glm::mat4>(), glm::radians(m_CameraRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), m_CameraPosition) * pitch * yaw * roll;

			m_ViewMatrix = glm::inverse(transform);

			m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;
		};


		void CameraController::RecalculateProjectionMatrix() {

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
					glm::radians(m_CameraProp.FOV),
					m_CameraProp.AspectRatio,
					m_CameraProp.NearPlaneDistance,
					m_CameraProp.FarPlaneDistance);

				break;
			};

			}

			m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;


		};





		bool CameraController::OnMouseScrolled(MouseScrolledEvent& e){

			AYIN_PROFILE_FUNCTION();

			m_ZoomLevel -= e.GetYoffset() * 0.25f;// 减法，窗口范围越小，看见的物体越大
			m_ZoomLevel = std::clamp(m_ZoomLevel, 0.25f, 10.0f);
			
			return false;

		}

		bool CameraController::OnMouseScrolled(Timestep deltaTime) {

			m_ZoomLevel -= Input::GetScrollYoffset() * 4.0 * deltaTime;// 减法，窗口范围越小，看见的物体越大
			m_ZoomLevel = std::clamp(m_ZoomLevel, 0.25f, 10.0f);

			return false;

		}


		bool CameraController::OnMouseMidButtonPressed(MouseButtonPressedEvent& e){

			AYIN_PROFILE_FUNCTION();

			if(e.GetMouseButton() == AYIN_MOUSE_BUTTON_MIDDLE){
				m_isRotate = true;
			}

			return false;

		}

		bool CameraController::OnMouseMidButtonReleased(MouseButtonReleasedEvent& e){

			AYIN_PROFILE_FUNCTION();

			if(e.GetMouseButton() == AYIN_MOUSE_BUTTON_MIDDLE){
				m_isRotate = false;
			}

			return false;
		}

		bool CameraController::OnWindowResize(WindowResizeEvent& e){

			AYIN_PROFILE_FUNCTION();

			if (e.GetWidth() <= 0 || e.GetHeight() <= 0) {
				return false;
			}

			m_CameraProp.AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();

			RecalculateProjectionMatrix();

			return false;

		}

	
		void CameraController::OnAxisKeyPressed( Timestep deltaTime ) {

			AYIN_PROFILE_FUNCTION();

			//原点-》平移量 -》逆矩阵

			glm::vec3 distance{ 0.0f };

			//! 因为OpenGL对相机的可是朝向是z的负半轴，所以，这里的加减关系和直觉上是相反的
			if (Input::IsKeyPressed(AYIN_KEY_A)) { distance.x -= m_CameraTranslateSpeed * deltaTime; };
			if (Input::IsKeyPressed(AYIN_KEY_D)) { distance.x += m_CameraTranslateSpeed * deltaTime; };
			if (Input::IsKeyPressed(AYIN_KEY_W)) { distance.z -= m_CameraTranslateSpeed * deltaTime; };
			if (Input::IsKeyPressed(AYIN_KEY_S)) { distance.z += m_CameraTranslateSpeed * deltaTime; };

			if (glm::length(distance) > 0.0f) {

				//! 原点 -> 平移量 -> 逆矩阵
				//让移动基于相机的本地坐标，当然GetRotationMatrix()是我临时加的，之后可能会移动到Transform中
				distance = GetRotationMatrix() * glm::translate(glm::identity<glm::mat4>(), distance) * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f};

				m_CameraPosition += glm::normalize(distance);
				AYIN_ERROR("direction:{0}, {1}, {2}", distance.x, distance.y, distance.z);


				SetPosition(m_CameraPosition);

			}

		}

		void CameraController::OnMouseMoved( Timestep deltaTime ) {

			AYIN_PROFILE_FUNCTION();

			//正交相机（我们使用按键）或者非旋转状态无法使用鼠标发出旋转指令
			if (m_CameraProp.Type == Camera::CameraType::Orthogonal || !m_isRotate) {
				return;
			}

			glm::vec2 position = Ayin::Input::GetMousePosition();

			position.x = (position.x - 640);
			position.y = (position.y - 360);

			static glm::vec3 lastRotation{ position.y, position.x, 0.0f };
			glm::vec3 rotation{ 0.0f };


			rotation = glm::vec3{ position.y, position.x, 0.0f };

			//死区半径
			if (glm::length(rotation) > 50.0f) {

				m_CameraRotation -= glm::normalize(rotation) * m_CameraRotationSpeed  * float(deltaTime);

				SetRotation(m_CameraRotation);

				lastRotation = { position.y, position.x, 0.0f };

			}


		}


}
