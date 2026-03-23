#include <AyinPch.h>

#include "Renderer.h"
#include <Platform/OpenGL/OpenGLShader.h>

namespace Ayin {

	Renderer::SceneData* Renderer::s_SceneData = new SceneData{ .projectionViewMatrix = {glm::identity<glm::mat4>()}};

	void Renderer::BeginScene(const Camera& rendererCamera) {

		s_SceneData->projectionViewMatrix = rendererCamera.GetProjecttionViewMatrix();

	}


	void Renderer::EndScene() {}

	/// <summary>
	/// 对指定顶点数组使用指定shader（材质）进行渲染
	/// </summary>
	/// <param name="shader"></param>
	/// <param name="vertexArray"></param>
	void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const Ref<UniformBuffer>& UBO) {

		shader->Bind();//在一些渲染API中，要求绑定VAO之前就必须有一个着色器（在创建和设置VAO时也是如此），以保证布局相对应（OpenGL没有这个限制），所以我们写在开头位置
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ProjectionViewMatrix", s_SceneData->projectionViewMatrix);

		UBO->Bind();

		vertexArray->Bind();

		RenderCommand::DrawIndexed(vertexArray);

		shader->UnBind();
		UBO->UnBind();
		vertexArray->UnBind();
	}

}