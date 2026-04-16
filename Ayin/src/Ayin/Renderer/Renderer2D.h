#pragma once


#include "Ayin/Core/Core.h"
#include "Ayin/Renderer/VertexArray.h"
#include "Ayin/Renderer/Shader.h"
#include "Ayin/Renderer/Camera.h"
#include "Ayin/Renderer/Texture.h"

#include <glm/glm.hpp>


namespace Ayin {


	struct Renderer2DStroage {

		Ref<VertexArray> QuadVAO;			//默认矩形

		Ref<Shader> QuadShader;				//默认Shader

		Ref<Texture2D> WhiteTexture;		//默认纹理

	};

	AYIN_API class Renderer2D {

	public:

		static void Init();

		static void Shutdown();

		static void BeginScene(const Camera& camera);
		static void EndScene();
		

		static void DrawQuad(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& size);
		static void DrawQuad(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& size, const Ref<Texture2D>& texture);

	private:

		static Renderer2DStroage* s_Data;

	};

}