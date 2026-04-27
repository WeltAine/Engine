#include <AyinPch.h>

#include "Platform/OpenGL/OpenGLFramebuffer.h"

#include "Glad/glad.h"


namespace Ayin {


	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& specification) 
		: Framebuffer{specification}
	{

		Init();

	}

	OpenGLFramebuffer::~OpenGLFramebuffer() {

		glDeleteFramebuffers(1, &m_FramebufferID);
		glDeleteTextures(1, &m_ColorAttachment);
		glDeleteRenderbuffers(1, &m_DepthAndStencilAttachment);


	}


	void OpenGLFramebuffer::Init() {

		if (m_FramebufferID != 0) {

			glDeleteFramebuffers(1, &m_FramebufferID);
			glDeleteTextures(1, &m_ColorAttachment);
			glDeleteRenderbuffers(1, &m_DepthAndStencilAttachment);

			m_ColorAttachment = 0;
			m_DepthAndStencilAttachment = 0;

		}

		glCreateFramebuffers(1, &m_FramebufferID);

		switch (m_FramebufferSpecification.Samples) {

			case(1): {

				//颜色附件（0号位附件）
				glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachment);
				glTextureStorage2D(m_ColorAttachment, 1, GL_RGBA8, m_FramebufferSpecification.Width, m_FramebufferSpecification.Height);

				//参数设置
				glTextureParameteri(m_ColorAttachment, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//缩小时是使用临近的四个像素加权平均
				glTextureParameteri(m_ColorAttachment, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glTextureParameteri(m_ColorAttachment, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTextureParameteri(m_ColorAttachment, GL_TEXTURE_WRAP_T, GL_REPEAT);

				glNamedFramebufferTexture(m_FramebufferID, GL_COLOR_ATTACHMENT0, m_ColorAttachment, 0);//多重采样纹理是没有minmip的，所以最后一个参数选0


				//深度和模板附件（采用RBO）
				glCreateRenderbuffers(1, &m_DepthAndStencilAttachment);
				glNamedRenderbufferStorage(
					m_DepthAndStencilAttachment,
					GL_DEPTH24_STENCIL8,
					m_FramebufferSpecification.Width,
					m_FramebufferSpecification.Height
				);

				glNamedFramebufferRenderbuffer(m_FramebufferID, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthAndStencilAttachment);
				glNamedFramebufferRenderbuffer(m_FramebufferID, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthAndStencilAttachment);
				break;

			}

			default: {

				//颜色附件（0号位附件）
				glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &m_ColorAttachment);
				glTextureStorage2DMultisample(
					m_ColorAttachment,							//纹理
					m_FramebufferSpecification.Samples,			//采样数（虚拟分辨率）
					GL_RGBA8,									//纹理格式
					m_FramebufferSpecification.Width,			//纹理宽度
					m_FramebufferSpecification.Height,			//纹理高度
					GL_TRUE										//使用硬件厂商设定的采样点布局
				);

				//! 多重采样纹理是不需要设置纹理参数的

				glNamedFramebufferTexture(m_FramebufferID, GL_COLOR_ATTACHMENT0, m_ColorAttachment, 0);//多重采样纹理是没有minmip的，所以最后一个参数选0


				//深度和模板附件（采用RBO）
				glCreateRenderbuffers(1, &m_DepthAndStencilAttachment);
				glNamedRenderbufferStorageMultisample(
					m_DepthAndStencilAttachment,
					m_FramebufferSpecification.Samples,
					GL_DEPTH24_STENCIL8,
					m_FramebufferSpecification.Width,
					m_FramebufferSpecification.Height
				);

				glNamedFramebufferRenderbuffer(m_FramebufferID, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthAndStencilAttachment);
				glNamedFramebufferRenderbuffer(m_FramebufferID, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthAndStencilAttachment);
				break;
			}

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

		m_FramebufferSpecification.Width = width;
		m_FramebufferSpecification.Height = height;

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
	
		FramebufferSpecification specification{ .Width{m_FramebufferSpecification.Width},
												.Height{m_FramebufferSpecification.Height},
												.Samples = 1};
		OpenGLFramebuffer fbo(specification);


		//! 因为一个帧缓冲可以有多个颜色附件，所需需要指定是那个颜色附件和那个颜色附件之间进行传递
		glNamedFramebufferReadBuffer(m_FramebufferID, GL_COLOR_ATTACHMENT0);
		glNamedFramebufferDrawBuffer(fbo.m_FramebufferID, GL_COLOR_ATTACHMENT0);

		glBlitNamedFramebuffer(
			m_FramebufferID, fbo.m_FramebufferID,
			0, 0, m_FramebufferSpecification.Width, m_FramebufferSpecification.Height,
			0, 0, m_FramebufferSpecification.Width, m_FramebufferSpecification.Height,
			GL_COLOR_BUFFER_BIT, 
			GL_LINEAR
		);

		//! 而帧缓冲只有一个模板和深度附件，不用glNamedFramebufferReadBuffer来定位，最后一个参数必须是GL_NEAREST
		glBlitNamedFramebuffer(
			m_FramebufferID, fbo.m_FramebufferID,
			0, 0, m_FramebufferSpecification.Width, m_FramebufferSpecification.Height,
			0, 0, m_FramebufferSpecification.Width, m_FramebufferSpecification.Height,
			GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, 
			GL_NEAREST
		);


		Ref<Texture2D> texture = Texture2D::Create(m_FramebufferSpecification.Width, m_FramebufferSpecification.Height);
		glCopyImageSubData(
			fbo.m_ColorAttachment, GL_TEXTURE_2D, 0, 
			0, 0, 0, 
			*texture, GL_TEXTURE_2D, 0, 
			0, 0, 0,
			m_FramebufferSpecification.Width, m_FramebufferSpecification.Height, 1
			);

		return texture;
	
	};


}