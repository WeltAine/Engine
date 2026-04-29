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

		uint32_t m_FramebufferID = 0, m_ResolveFramebufferID = 0;	//帧缓冲和解析帧缓冲
		//! 我们的帧缓冲允许多重采样，而多重采样的纹理没法直接输出到屏幕上，所以我们需要将多重采样纹理转换成普通纹理
		//! OpenGL中的流程时需要通过不同采样规格的帧缓冲来实现双向的解析，所以我们需要一个解析缓冲罐其采样数为1

		//x 因为我们的Texture纹理暂时不支持动态尺寸的纹理，而且设计上还太简单不支持格式参数（这需要使用Image的API，或者说我们可以在调整窗口时销毁，之后重建）
		// 已经改进Texture，基于适配器某事添加通用的构造方法
		Ref<Texture2D> m_ColorAttachmentTexture;			//颜色附件
		Ref<Texture2D> m_ResolveAttachmentTexture;			//解析帧缓冲附件（Sample > 1 时使用）

		uint32_t m_DepthAndStencilAttachment = 0;			//深度和模板附件
		

	};

}