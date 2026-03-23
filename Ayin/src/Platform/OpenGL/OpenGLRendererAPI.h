#pragma once

#include "Ayin/Renderer/RendererAPI.h"


namespace Ayin {

	
	class OpenGLRendererAPI : public RendererAPI {

	public:
		
		// 通过 RendererAPI 继承
		void SetClearColor(const glm::vec4& color) const override;

		void Clear() const override;

		void DrawIndexed(const Ref<VertexArray>& vertexArray) const override;

	};

}