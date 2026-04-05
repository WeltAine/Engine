#pragma once

#include "Ayin/Core.h"

#include "Ayin/Event/KeyEvent.h"
#include "Ayin/Event/MouseEvent.h"

#include "Ayin/Core/Timestep.h"

#include "Ayin/Renderer/Camera.h"


namespace Ayin{

    class AYIN_API CameraController{

    public:
        CameraController(const CameraProp& cameraProp);

        void OnUpdate(Timestep deltaTime);
        void OnEvent(Event& e);

        inline const Camera& GetCamera() const { return m_Camera; };
        inline const CameraProp& GetCameraProp() const { return m_CameraProp; };

    private:
        bool OnMouseScrolled(MouseSrolledEvent& e);
        bool OnMouseMidButtonPressed(MouseButtonPressedEvent& e);
        bool OnMouseMidButtonReleased(MouseButtonReleaseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

        void OnAxisKeyPressed( Ayin::Timestep deltaTime );
        void OnMouseMoved( Ayin::Timestep deltaTime );
        


    private:
        Camera m_Camera;

        CameraProp m_CameraProp;

        bool m_isRotate = false;                                                //是否开启旋转

        float m_ZoomLevel = 1.0f;                                               //缩放

        glm::vec3 m_CameraPosition = {0.0f, 0.0f, 1.0f};						// 位置
        glm::vec3 m_CameraRotation = {0.0f, 0.0f, 0.0f};						// 角度

        float m_CameraTranslateSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;

    }


}