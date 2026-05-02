#pragma once

#include <Ayin.h>

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

	Ayin::CameraController m_CameraController;		//相机控制器

	Ayin::Ref<Ayin::Texture2D> m_Texture;			//测试用纹理

	Ayin::Ref<Ayin::Framebuffer> m_Framebuffer;		//游戏视窗帧缓冲

	Ayin::Ref<Ayin::Texture2D> m_ViewTexture;		//游戏视窗帧缓冲对应纹理

	glm::i32vec2 m_ViewportSize = {0.0f, 0.0f};		//游戏视窗大小

	Ayin::Scene m_ActiveScene;						//ECS场景测试
	Ayin::Entity m_TextureEntity;

};