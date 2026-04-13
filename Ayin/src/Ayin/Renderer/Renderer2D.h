#pragma once

#include <GLM/glm.hpp>

#include "Ayin/Core.h"
#include "Ayin/Renderer/VertexArray.h"
#include "Ayin/Renderer/Shader.h"
#include "Ayin/Renderer/Camera.h"



namespace Ayin {


	struct Renderer2DStroage {

		Ref<VertexArray> QuadVAO;
		Ref<Shader> QuadShader;

	};

	class Renderer2D {

	public:

		static void Init();

		static void Shutdown();

		static void BeginScene(const Camera& camera);
		static void EndScene();
		

		static void DrawQuad(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& size);

	private:

		static Renderer2DStroage* s_Data;

	};

}