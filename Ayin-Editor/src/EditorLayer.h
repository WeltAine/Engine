#pragma once

#include <Ayin.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/PropertiesPanel.h"

#include <ImGuizmo.h>

class EditorLayer : public Ayin::Layer {

public:

	EditorLayer();

	virtual ~EditorLayer() = default;

	virtual void OnAttach() override;//加入层栈
	virtual void OnDetach() override;//退出层栈

	virtual void OnUpdate(Ayin::Timestep deltaTime) override;//层更新

	virtual void OnImGuiRender() override;

	virtual void OnEvent(Ayin::Event& event) override;

private:

	//快捷键
	bool OnKeyPressed(Ayin::KeyPressedEvent& event);

	//场景导入导出
	void OpenScene();
	void NewScene();
	void SaveScene();

private:

	Ayin::CameraController m_CameraController;		//相机控制器

	Ayin::Ref<Ayin::Texture2D> m_Texture;			//测试用纹理

	Ayin::Ref<Ayin::Framebuffer> m_Framebuffer;		//游戏视窗帧缓冲

	Ayin::Ref<Ayin::Texture2D> m_ViewTexture;		//游戏视窗帧缓冲对应纹理

	glm::i32vec2 m_SceneSize = {0.0f, 0.0f};		//Viewport中场景Image/Framebuffer尺寸

	ImGuizmo::MODE m_GizmoMode = ImGuizmo::MODE::LOCAL;
	ImGuizmo::OPERATION m_GizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	ImGuizmo::MOVETYPE m_GizmoMoveType = ImGuizmo::MOVETYPE::MT_NONE;//不确定，好像没用上

	//ECS测试
	Ayin::Ref<Ayin::Scene> m_ActiveScene;						
	//x Ayin::Entity m_TextureEntity;
	Ayin::Entity m_SceneCamera;

//面板测试
	Ayin::SceneHierarchyPanel m_SceneHierarchyPanel;
	Ayin::PropertiesPanel m_PropertiesPanel;

};





class CameraControllerScript : public Ayin::ScriptableEntity {

public:
	AYIN_CUSTOM_SCRIPT_TYPE(CameraControllerScript);

public:
	CameraControllerScript() = default;
	~CameraControllerScript() = default;

	virtual void OnUpdate(Ayin::Timestep deltaTime) override {

		AYIN_PROFILE_FUNCTION();

		OnAxisKeyPressed(deltaTime);
		OnMouseMoved(deltaTime);
		OnMouseScrolled(deltaTime);

		auto&& [transfrom, camera] = GetComponents < Ayin::TransformComponent, Ayin::CameraComponent >();
		camera.Camera.SetViewMatrix(transfrom.Position, transfrom.Rotation);
	}

private:

	bool OnMouseScrolled(Ayin::Timestep deltaTime) {

		auto& cameraComponent = GetComponents<Ayin::CameraComponent>();
		float zoomLevel = cameraComponent.Camera.GetCameraZoomLevel();

		zoomLevel -= Ayin::Input::GetScrollYoffset() * 4.0 * deltaTime;// 减法，窗口范围越小，看见的物体越大

		cameraComponent.Camera.SetCameraZoomLevel(zoomLevel);

		return false;

	}
	void OnAxisKeyPressed(Ayin::Timestep deltaTime) {

		AYIN_PROFILE_FUNCTION();

		auto&& [cameraComponent, cameraTransform] = GetComponents<Ayin::CameraComponent, Ayin::TransformComponent>();

		//原点-》平移量 -》逆矩阵
		glm::vec3 distance{ 0.0f };

		//! 因为OpenGL对相机的可是朝向是z的负半轴，所以，这里的加减关系和直觉上是相反的
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_A)) { distance.x -= m_CameraTranslateSpeed * deltaTime; };
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_D)) { distance.x += m_CameraTranslateSpeed * deltaTime; };
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_W)) { distance.z -= m_CameraTranslateSpeed * deltaTime; };
		if (Ayin::Input::IsKeyPressed(AYIN_KEY_S)) { distance.z += m_CameraTranslateSpeed * deltaTime; };

		if (glm::length(distance) > 0.0f) {

			//! 原点 -> 平移量 -> 逆矩阵
			//让移动基于相机的本地坐标，当然GetRotationMatrix()是我临时加的，之后可能会移动到Transform中
			distance = cameraTransform.GetRotationMatrix() * glm::translate(glm::identity<glm::mat4>(), distance) * glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };

			cameraTransform.Position += glm::normalize(distance);
			AYIN_ERROR("direction:{0}, {1}, {2}", distance.x, distance.y, distance.z);

		}

	}

	void OnMouseMoved(Ayin::Timestep deltaTime) {

		AYIN_PROFILE_FUNCTION();

		auto& cameraTransform = GetComponents <Ayin::TransformComponent>();

		auto& cameraProp = GetComponents <Ayin::CameraComponent>().Camera.GetCameraProp();

		//正交相机（我们使用按键）或者非旋转状态无法使用鼠标发出旋转指令
		if (cameraProp.Type == Ayin::Camera::CameraType::Orthogonal || !m_isRotate) {
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

			cameraTransform.Rotation -= glm::normalize(rotation) * m_CameraRotationSpeed * float(deltaTime);


			lastRotation = { position.y, position.x, 0.0f };

			AYIN_ERROR("Rotate");
			AYIN_ERROR("{0}, {1}, {2}", cameraTransform.Rotation.x, cameraTransform.Rotation.y, cameraTransform.Rotation.z);

		}


	}



private:

	bool m_isRotate = false;                                                //是否开启旋转

	float m_CameraTranslateSpeed = 1.0f, m_CameraRotationSpeed = 120.0f;

public:
	virtual void OnGui() override {

		ImGui::SeparatorText("Camera Controller");
		ImGui::DragFloat("Translate Speed", &m_CameraTranslateSpeed, 0.01f);
		ImGui::DragFloat("Rotation Speed", &m_CameraRotationSpeed, 1.0f);
		ImGui::Checkbox("Enable Rotation", &m_isRotate);

	};

public:
	struct glaze {

		using T = CameraControllerScript;

		static constexpr auto value = glz::object(
			"Translate Speed", &T::m_CameraTranslateSpeed,
			"Rotation Speed", &T::m_CameraRotationSpeed,
			"Enable Rotation", &T::m_isRotate
		);

	};
	

};
AYIN_SCRIPT(CameraControllerScript);


