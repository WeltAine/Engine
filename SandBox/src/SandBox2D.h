#pragma once

#include <Ayin.h>

class SandBox2D : public Ayin::Layer {

public:

	SandBox2D();

	inline ~SandBox2D() = default;

	virtual void OnAttach() override;//加入层栈
	virtual void OnDetach() override;//退出层栈

	virtual void OnUpdate(Ayin::Timestep deltaTime) override;//层更新

	virtual void OnImGuiRender() override;

	virtual void OnEvent(Ayin::Event& event) override;


private:

	Ayin::CameraController m_CamreaController;		//相机控制器

	Ayin::Ref<Ayin::Texture2D> m_Texture;			//测试用纹理
};