#include <AyinPch.h>

#include "Ayin/CameraController.h"
#include "Ayin/KayCodes.h"
#include "Ayin/MouseButtonCodes.h"
#include "Ayin/Input.h"

namespace Ayin{


        CameraController::CameraController(const CameraProp& cameraProp)
            :m_Camera{cameraProp}, m_CameraProp{cameraProp}
        {
            AYIN_ERROR("Now FOV: {0}", m_CameraProp.FOV);


            // 同步一下相机状态（相机控制器类和相机控制类的默认值可能不同）
            m_Camera.SetPosition(m_CameraPosition);
            m_Camera.SetRotation(m_CameraRotation);

        }

        void CameraController::OnUpdate(Timestep deltaTime){

            OnAxisKeyPressed(deltaTime);
            OnMouseMoved(deltaTime);

        }


        void CameraController::OnEvent(Event& e){

            EventDispatcher dispatcher{ e };

            dispatcher.Dispatch<MouseSrolledEvent>(BIND_EVENT_FUN(CameraController::OnMouseScrolled));
            dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FUN(CameraController::OnMouseMidButtonPressed));
            dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FUN(CameraController::OnMouseMidButtonReleased));
            dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FUN(CameraController::OnWindowResize));

        }

        bool CameraController::OnMouseScrolled(MouseSrolledEvent& e){

            m_ZoomLevel -= e.GetYoffset() * 0.25f;// 减法，窗口范围越小，看见的物体越大
            m_ZoomLevel = std::clamp(m_ZoomLevel, 0.25f, 10.0f);


            float height = m_CameraProp.Height * m_ZoomLevel;
            float FOV = 2 * glm::degrees( std::atan(m_ZoomLevel * std::tan(glm::radians(0.5f * m_CameraProp.FOV))) );
            

            m_Camera.SetProjection({ 
                .Type{m_CameraProp.Type},
                .FOV{FOV}, .Height{height},
                .AspectRatio{m_CameraProp.AspectRatio},
                .NearPlaneDistance{m_CameraProp.NearPlaneDistance}, .FarPlaneDistance{m_CameraProp.FarPlaneDistance} });


            return false;

        }

        bool CameraController::OnMouseMidButtonPressed(MouseButtonPressedEvent& e){

            if(e.GetMouseButton() == AYIN_MOUSE_BUTTON_MIDDLE){
                m_isRotate = true;
            }

            return false;

        }

        bool CameraController::OnMouseMidButtonReleased(MouseButtonReleasedEvent& e){

            if(e.GetMouseButton() == AYIN_MOUSE_BUTTON_MIDDLE){
                m_isRotate = false;
            }

            return false;
        }

        bool CameraController::OnWindowResize(WindowResizeEvent& e){

            if (e.GetWidth() <= 0 || e.GetHeight() <= 0) {
                return false;
            }

            m_CameraProp.AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();


            float height = m_CameraProp.Height * m_ZoomLevel;
            float FOV = 2 * glm::degrees(std::atan(m_ZoomLevel * std::tan(glm::radians(0.5f * m_CameraProp.FOV))));

            m_Camera.SetProjection({
                .Type{m_CameraProp.Type},
                .FOV{FOV}, .Height{height},
                .AspectRatio{m_CameraProp.AspectRatio},
                .NearPlaneDistance{m_CameraProp.NearPlaneDistance}, .FarPlaneDistance{m_CameraProp.FarPlaneDistance} });


            return false;

        }

    
        void CameraController::OnAxisKeyPressed( Ayin::Timestep deltaTime ) {
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
                distance = m_Camera.GetRotationMatrix() * glm::translate(glm::identity<glm::mat4>(), distance) * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f};

                m_CameraPosition += glm::normalize(distance);
                AYIN_ERROR("direction:{0}, {1}, {2}", distance.x, distance.y, distance.z);


                m_Camera.SetPosition(m_CameraPosition);

                //AYIN_ERROR("Move");
                //AYIN_ERROR("{0}, {1}, {2}", m_CameraRotation.x, m_CameraRotation.y, m_CameraRotation.z);

            }

        }

        void CameraController::OnMouseMoved( Ayin::Timestep deltaTime ) {

            //正交相机（我们使用按键）或者非旋转状态无法使用鼠标发出旋转指令
            if (m_CameraProp.Type == Camera::CameraType::Orthogonal || !m_isRotate) {
                return;
            }

            auto [x, y] = Ayin::Input::GetMousePosition();

            x = (x - 640);
            y = (y - 360);

            static glm::vec3 lastRotation{ y, x, 0.0f };
            glm::vec3 rotation{ 0.0f };


            rotation = glm::vec3{ y, x, 0.0f };

            //死区半径
            if (glm::length(rotation) > 50.0f) {

                m_CameraRotation -= glm::normalize(rotation) * m_CameraRotationSpeed  * float(deltaTime);

                m_Camera.SetRotation(m_CameraRotation);

                lastRotation = { y, x, 0.0f };

                AYIN_ERROR("Rotate");
                AYIN_ERROR("{0}, {1}, {2}", m_CameraRotation.x, m_CameraRotation.y, m_CameraRotation.z);

            }


        }


}