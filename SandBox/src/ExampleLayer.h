#pragma once

#include <Ayin.h>


class ExampleLayer : public Ayin::Layer {

public:
	ExampleLayer();
		
	virtual ~ExampleLayer() override = default;

	virtual void OnUpdate(Ayin::Timestep deltaTime) override;

	virtual void OnEvent(Ayin::Event& e) override;

	virtual void OnImGuiRender() override;



private:

	Ayin::ShaderLibrary m_ShaderLibrary;
	Ayin::Ref<Ayin::Shader> m_TextureShader;										//着色器程序
	Ayin::Ref<Ayin::Texture2D> m_Texture, m_BlendTexture;							//纹理
	Ayin::Ref<Ayin::UniformBuffer> m_UBO, m_BlendUBO;								//统一变量缓冲
	glm::mat4 m_Transform{ 1.0f };	// mode测试
	glm::vec3 m_ColorOffset{ 0.0f };// 其余uniform参数设置
	Ayin::Ref<Ayin::VertexArray> m_TriangleVertexArray, m_SquareVertexArray;		//顶点数组

	Ayin::CameraController m_SceneCameraController;									//场景相机


};
