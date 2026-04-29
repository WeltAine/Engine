#pragma once

#include <Ayin.h>

class EditorLayer : public Ayin::Layer {

public:

	EditorLayer();

	inline ~EditorLayer() = default;

	virtual void OnAttach() override;//加入层栈
	virtual void OnDetach() override;//退出层栈

	virtual void OnUpdate(Ayin::Timestep deltaTime) override;//层更新

	virtual void OnImGuiRender() override;

	virtual void OnEvent(Ayin::Event& event) override;


private:

	Ayin::CameraController m_CameraController;		//相机控制器

	Ayin::Ref<Ayin::Texture2D> m_Texture;			//测试用纹理

	Ayin::Ref<Ayin::Framebuffer> m_Framebuffer;

	Ayin::Ref<Ayin::Texture2D> m_ViewTexture;

	glm::vec2 m_ViewportSize = {0.0f, 0.0f};
};