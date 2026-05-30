#include <AyinPch.h>

#include "Platform/OpenGL/OpenGLFramebuffer.h"

#include "Glad/glad.h"

#include "Platform/OpenGL/OpenGLTexture.h"


namespace Ayin {


	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& specification) 
		: Framebuffer{specification}
	{

		Init();

	}

	OpenGLFramebuffer::~OpenGLFramebuffer() {

		uint32_t framebuffers[] = { m_FramebufferID , m_ResolveFramebufferID };
		glDeleteFramebuffers(2, framebuffers);
		glDeleteRenderbuffers(1, &m_DepthAndStencilAttachment);

	}


	void OpenGLFramebuffer::Init() {

		if (m_FramebufferID != 0 || m_ResolveFramebufferID != 0) {

			glDeleteRenderbuffers(1, &m_DepthAndStencilAttachment);

			uint32_t framebuffers[] = { m_FramebufferID , m_ResolveFramebufferID };
			glDeleteFramebuffers(2, framebuffers);
			glDeleteRenderbuffers(1, &m_DepthAndStencilAttachment);
		}

		glCreateFramebuffers(1, &m_FramebufferID);
		glCreateFramebuffers(1, &m_ResolveFramebufferID);

		uint32_t colorAttachment = 0;//颜色附件纹理

		switch (m_FramebufferSpecification.Samples) {

			case(1): {

				//颜色附件（0号位附件）
				glCreateTextures(GL_TEXTURE_2D, 1, &colorAttachment);
				glTextureStorage2D(colorAttachment, 1, GL_RGBA8, m_FramebufferSpecification.Size.x, m_FramebufferSpecification.Size.y);

				//参数设置
				glTextureParameteri(colorAttachment, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//缩小时是使用临近的四个像素加权平均
				glTextureParameteri(colorAttachment, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glTextureParameteri(colorAttachment, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTextureParameteri(colorAttachment, GL_TEXTURE_WRAP_T, GL_REPEAT);

				m_ColorAttachmentTexture = std::make_shared<OpenGLTexture2D>(colorAttachment, m_FramebufferSpecification.Size.x, m_FramebufferSpecification.Size.y);

				glNamedFramebufferTexture(m_FramebufferID, GL_COLOR_ATTACHMENT0, colorAttachment, 0);

				

				//深度和模板附件（采用RBO）
				glCreateRenderbuffers(1, &m_DepthAndStencilAttachment);
				glNamedRenderbufferStorage(
					m_DepthAndStencilAttachment,
					GL_DEPTH32F_STENCIL8,
					m_FramebufferSpecification.Size.x,
					m_FramebufferSpecification.Size.y
				);

				glNamedFramebufferRenderbuffer(m_FramebufferID, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthAndStencilAttachment);
				glNamedFramebufferRenderbuffer(m_FramebufferID, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthAndStencilAttachment);


				//解析帧缓冲
				m_ResolveAttachmentTexture = m_ColorAttachmentTexture;
				glNamedFramebufferTexture(m_ResolveFramebufferID, GL_COLOR_ATTACHMENT0, *m_ResolveAttachmentTexture, 0);

				break;

			}

			default: {

				//颜色附件（0号位附件）
				glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &colorAttachment);
				glTextureStorage2DMultisample(
					colorAttachment,							//纹理
					m_FramebufferSpecification.Samples,			//采样数（虚拟分辨率）
					GL_RGBA8,									//纹理格式
					m_FramebufferSpecification.Size.x,			//纹理宽度
					m_FramebufferSpecification.Size.y,			//纹理高度
					GL_TRUE										//使用硬件厂商设定的采样点布局
				);

				//! 多重采样纹理是不需要设置纹理参数的
				m_ColorAttachmentTexture = std::make_shared<OpenGLTexture2D>(colorAttachment, m_FramebufferSpecification.Size.x, m_FramebufferSpecification.Size.y);
				

				glNamedFramebufferTexture(m_FramebufferID, GL_COLOR_ATTACHMENT0, colorAttachment, 0);//多重采样纹理是没有minmip的，所以最后一个参数选0


				//深度和模板附件（采用RBO）
				glCreateRenderbuffers(1, &m_DepthAndStencilAttachment);
				glNamedRenderbufferStorageMultisample(
					m_DepthAndStencilAttachment,
					m_FramebufferSpecification.Samples,
					GL_DEPTH32F_STENCIL8,
					m_FramebufferSpecification.Size.x,
					m_FramebufferSpecification.Size.y
				);

				glNamedFramebufferRenderbuffer(m_FramebufferID, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthAndStencilAttachment);
				glNamedFramebufferRenderbuffer(m_FramebufferID, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthAndStencilAttachment);


				//解析帧缓冲
				//附件
				m_ResolveAttachmentTexture = Texture2D::Create(m_FramebufferSpecification.Size.x, m_FramebufferSpecification.Size.y);

				glNamedFramebufferTexture(m_ResolveFramebufferID, GL_COLOR_ATTACHMENT0, *m_ResolveAttachmentTexture, 0);

				break;
			}

			//! combind格式
			//	   格式				深度精度			模板精度			典型用途
			//	GL_DEPTH24_STENCIL8	24 - bit int		8 - bit int		通用，大多数场景
			//	GL_DEPTH32F_STENCIL8	32 - bit float	8 - bit int		高精度深度需要（大场景）
			//	两种都是 combined 格式——一块缓冲区物理上包含两种数据，GPU 按固定位偏移读写。Texture 要使用 combined 格式需要 OpenGL 4.3 + 或 ARB_stencil_texturing 扩展。

			//? RGBA8 能当深度 / 模板附件用吗？
			//!	不能。 深度和模板是 GPU 专用的存储器类型，和颜色缓冲区物理上不同。
			//	RGBA8  → 颜色附件(GPU 颜色压缩单元，按 RGBA 通道解释)
			//	D24S8  → 深度 / 模板附件(GPU 深度测试单元，按 24bit深度 + 8bit模板解释)
			//!	深度缓冲区有专门的硬件单元做深度比较（glDepthFunc）、Early - Z 剔除、Hi - Z 层级优化。拿 RGBA8 当深度附件绑，glCheckFramebufferStatus 直接返回 GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT。
		
			//? 分别绑定两个纹理到深度和模板附件？
			//		   可以。 这也是合法做法：
			/*	   // 深度纹理
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
				  // 模板纹理（独立一张）
				glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX8, w, h, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, nullptr);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencilTex, 0);
			*/
		
		}

		GLenum status = glCheckNamedFramebufferStatus(m_FramebufferID, GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			switch (status) {
				case GL_FRAMEBUFFER_UNDEFINED:						AYIN_CORE_ERROR("FBO Error: Undefined");				break;
				case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			AYIN_CORE_ERROR("FBO Error: Incomplete Attachment");	break;//这里被触发
				case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	AYIN_CORE_ERROR("FBO Error: Missing Attachment");		break;
				case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:         AYIN_CORE_ERROR("FBO Error: Incomplete Draw Buffer");	break;
				case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:         AYIN_CORE_ERROR("FBO Error: Incomplete Read Buffer");	break;
				case GL_FRAMEBUFFER_UNSUPPORTED:                    AYIN_CORE_ERROR("FBO Error: Unsupported Format");		break;
				case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:         AYIN_CORE_ERROR("FBO Error: Incomplete Multisample");	break;
				default:											AYIN_CORE_ERROR("FBO Error: Unknown error");
			}
			AYIN_CORE_ASSERT(false, "Framebuffer is incomplete!");
		}

	}


	void OpenGLFramebuffer::Resize(int width, int height) {

		m_FramebufferSpecification.Size.x = width;
		m_FramebufferSpecification.Size.y = height;

		Init();

	};


	void OpenGLFramebuffer::Invalidate() {



	}

	void OpenGLFramebuffer::Bind()  {
	
		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);

	};
	void OpenGLFramebuffer::UnBind() { 
		
		int currentFramebuffer = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFramebuffer);

		if (currentFramebuffer == m_FramebufferID)
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	};


	Ref<Texture2D> OpenGLFramebuffer::GetColorAttachment(int index) const {
		

		if (m_FramebufferSpecification.Samples > 1) {
			
			//配置要读和写入的buffer
			glNamedFramebufferReadBuffer(m_FramebufferID, GL_COLOR_ATTACHMENT0 + index);
			glNamedFramebufferDrawBuffer(m_ResolveFramebufferID, GL_COLOR_ATTACHMENT0);

			//传输
			glBlitNamedFramebuffer(
				m_FramebufferID, m_ResolveFramebufferID,
				0, 0, m_FramebufferSpecification.Size.x, m_FramebufferSpecification.Size.y,
				0, 0, m_FramebufferSpecification.Size.x, m_FramebufferSpecification.Size.y,
				GL_COLOR_BUFFER_BIT,
				GL_LINEAR
			);

		}

		return m_ResolveAttachmentTexture;
	
	};


}
