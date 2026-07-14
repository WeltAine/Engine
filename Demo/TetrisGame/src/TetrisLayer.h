#pragma once

#include <Ayin.h>

class TetrisLayer : public Ayin::Layer {

public:
	TetrisLayer();
	~TetrisLayer() override = default;

	void OnAttach() override;
	void OnUpdate(Ayin::Timestep deltaTime) override;
	void OnImGuiRender() override;
	void OnEvent(Ayin::Event& event) override;

private:
	void ResetScene();
	bool OnWindowResize(Ayin::WindowResizeEvent& event);

private:
	Ayin::Ref<Ayin::Scene> m_Scene;
	glm::vec4 m_ClearColor{ 0.03f, 0.035f, 0.055f, 1.0f };

};
