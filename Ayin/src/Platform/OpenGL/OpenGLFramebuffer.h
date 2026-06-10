#pragma once



#include "Ayin/Renderer/Framebuffer.h"

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

		// 返回可被屏幕直接渲染的单采样（解析）纹理ID
		virtual uint32_t GetColorAttachmentRendererID(uint32_t attachmentIndex = 0) const override;

		// 按附件格式清理颜色附件，同时同步主FBO和解析FBO
		virtual void ClearColorAttachment(uint32_t attachmentIndex, PixelValue value) override;


		// 读取附件信息（目前仅支持颜色附件）
		virtual PixelValue ReadPixel(uint32_t attachmentIndex, int x, int y) const override;


	private:

		// 将指定颜色附件同步到解析FBO，并恢复本类维护的FBO读写配置
		bool ResolveAttachment(uint32_t attachmentIndex) const;


	private:

		uint32_t m_FramebufferID = 0, m_ResolveFramebufferID = 0;	//帧缓冲和解析帧缓冲
		//! 我们的帧缓冲允许多重采样，而多重采样的纹理没法直接输出到屏幕上，所以我们需要将多重采样纹理转换成普通纹理
		//! OpenGL中的流程时需要通过不同采样规格的帧缓冲来实现双向的解析，所以我们需要一个解析缓冲罐其采样数为1
		//! 帧缓冲默认使用第一个颜色附件

		//x 因为我们的Texture纹理暂时不支持动态尺寸的纹理，而且设计上还太简单不支持格式参数（这需要使用Image的API，或者说我们可以在调整窗口时销毁，之后重建）
		// 已经改进Texture，基于适配器某事添加通用的构造方法
		std::vector<uint32_t> m_ColorAttachments;			//所有颜色附件
		uint32_t m_DepthAndStencilAttachment = 0;			//深度和模板附件

		std::vector<uint32_t> m_ResolveColorAttachments;					//解析帧缓冲颜色附件
		uint32_t m_ResolveDepthAndStencilAttachment = 0;					//解析帧缓冲深度和模板附件
		

	};

}
