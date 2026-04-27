#pragma once



#include "Ayin/Renderer/Framebuffer.h"

#include "Ayin/Renderer/Texture.h"

namespace Ayin {


	class AYIN_API OpenGLFramebuffer : public Framebuffer {


	public:

		OpenGLFramebuffer(const FramebufferSpecification& specification);
		~OpenGLFramebuffer();

		void Init();

		virtual void Bind() override;
		virtual void UnBind() override;


		virtual void Resize(int width, int height) override;

		//附件失效
		void Invalidate();

		inline virtual operator uint32_t() { return m_FramebufferID; };

		virtual Ref<Texture2D> GetColorAttachment(int index = 0) const override;


	private:

		uint32_t m_FramebufferID = 0;

		//因为我们的Texture纹理暂时不支持动态尺寸的纹理，而且设计上还太简单不支持格式参数（这需要使用Image的API，或者说我们可以在调整窗口时销毁，之后重建）
		uint32_t m_ColorAttachment = 0, m_DepthAndStencilAttachment = 0; //颜色附件和深度附件

	};

}