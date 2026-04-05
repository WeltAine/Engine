#include <AyinPch.h>

#include "Ayin/CameraController.h"

namespace Ayin{


        CameraController::CameraController(const CameraProp& cameraProp)
            :m_Camera{cameraProp}, m_CameraProp{cameraProp}
        {
            // 同步一下数据
            m_Camera.SetPosition(m_CameraPosition);
            m_Camera.SetRotation(m_CameraRotation);

        }

        void OnUpdate(Timestep deltaTime){

            OnAxisKeyPressed(deltaTime);
            OnMouseMoved(deltaTime);

        }


        void OnEvent(Event& e){

            EventDispatcher dispatcher(e);

            e.Dispatch<MouseSrolledEvent>(BIND_EVENT_FUN(CameraController::OnMouseScrolled))
            e.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FUN(CameraController::OnMouseMidButtonPressed));
            e.Dispatch<MouseButtonReleaseEvent>(BIND_EVENT_FUN(CameraController::OnMouseMidButtonReleased));
            e.Dispatch<WindowResizeEvent>(BIND_EVENT_FUN(CameraController::OnWindowResize));

        }

        bool OnMouseScrolled(MouseSrolledEvent& e){

            m_ZoomLevel -= e.GetYoffset() * 0.25f;// 减法，窗口范围越小，看见的越大
            m_ZoomLevel = std::clamp(m_ZoomLevel, 0.25f, 10.0f);

            m_CameraProp.Height *= m_ZoomLevel;
            
            m_Camera.SetProjectiong(m_CameraProp);

            return false;

        }

        bool OnMouseMidButtonPressed(MouseButtonPressedEvent& e){

            if(e.GetMouseButton() == AYIN_MOUSE_BUTTON_MIDDLE){
                m_isRotate = true;
            }

            return false;

        }

        bool OnMouseMidButtonReleased(MouseButtonReleaseEvent& e){

            if(e.GetMouseButton() == AYIN_MOUSE_BUTTON_MIDDLE){
                m_isRotate = false;
            }

            return false;
        }

        bool OnWindowResize(WindowResize& e){

            m_CameraProp.AspectRatio = e.GetWidth() / e.GetHeight();

            m_Camera.SetProjection(m_CameraProp);

            return false;

        }

    
        void OnAxisKeyPressed( Ayin::Timestep deltaTime ) {
		    //原点-》平移量 -》逆矩阵

            glm::vec3 distance{ 0.0f };

            //! 因为OpenGL对相机的可是朝向是z的负半轴，所以，这里的加减关系和直觉上是相反的
            if (Ayin::Input::IsKeyPressed(AYIN_KEY_LEFT)) { distance.x -= m_MoveSpeed * deltaTime; };
            if (Ayin::Input::IsKeyPressed(AYIN_KEY_RIGHT)) { distance.x += m_MoveSpeed * deltaTime; };
            if (Ayin::Input::IsKeyPressed(AYIN_KEY_UP)) { distance.z -= m_MoveSpeed * deltaTime; };
            if (Ayin::Input::IsKeyPressed(AYIN_KEY_DOWN)) { distance.z += m_MoveSpeed * deltaTime; };

            if (glm::length(distance) > 0.0f) {

                //! 原点 -> 平移量 -> 逆矩阵
                //让移动基于相机的本地坐标，当然GetRotationMatrix()是我临时加的，之后可能会移动到Transform中
                distance = m_SceneCamera.GetRotationMatrix() * glm::translate(glm::identity<glm::mat4>(), distance) * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f};

                m_CameraPosition += glm::normalize(distance);
                AYIN_ERROR("direction:{0}, {1}, {2}", distance.x, distance.y, distance.z);


                m_SceneCamera.SetPosition(m_CameraPosition);

                //AYIN_ERROR("Move");
                //AYIN_ERROR("{0}, {1}, {2}", m_CameraRotation.x, m_CameraRotation.y, m_CameraRotation.z);

            }

        }

        void OnMouseMoved( Ayin::Timestep deltaTime ) {

            auto [x, y] = Ayin::Input::GetMousePosition();

            x = (x - 640);
            y = (y - 360);

            static glm::vec3 lastRotation{ y, x, 0.0f };
            glm::vec3 rotation{ 0.0f };


            rotation = glm::vec3{ y, x, 0.0f };


            if (glm::length(rotation) > 30.0f) {

                m_CameraRotation -= glm::normalize(rotation) * m_RotationSpeed  * float(deltaTime);

                m_SceneCamera.SetRotation(m_CameraRotation);

                lastRotation = { y, x, 0.0f };

                AYIN_ERROR("Rotate");
                AYIN_ERROR("{0}, {1}, {2}", m_CameraRotation.x, m_CameraRotation.y, m_CameraRotation.z);

            }


        }


}