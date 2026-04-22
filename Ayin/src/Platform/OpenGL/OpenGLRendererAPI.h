#pragma once

#include "Ayin/Renderer/RendererAPI.h"


namespace Ayin {

	
	class OpenGLRendererAPI : public RendererAPI {

	public:
		
		// 通过 RendererAPI 继承
		virtual void Init() override;

		virtual void SetViewport(int x, int y, int width, int height) override;

		virtual void SetClearColor(const glm::vec4& color) const override;

		virtual void Clear() const override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) const override;
		virtual void Draw2DIndexed(const Ref<VertexArray>& vertexArray, int count) const override;

	};

}