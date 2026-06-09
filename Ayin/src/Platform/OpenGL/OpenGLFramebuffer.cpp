#include <AyinPch.h>

#include "Platform/OpenGL/OpenGLFramebuffer.h"

#include "Glad/glad.h"

#include "Platform/OpenGL/OpenGLTexture.h"

namespace Ayin {

	namespace Utils {


		static bool IsColorAttachment(FramebufferAttachmentFormat format) {
			return format != FramebufferAttachmentFormat::Depth_Stencil && format != FramebufferAttachmentFormat::None;
		}


		static bool HasDepthStencilAttachment(const FramebufferAttachmentsSpecification& attachmentsSpecification) {

			for (FramebufferAttachmentFormat format : attachmentsSpecification.AttachmentFormats) {
				if (format == FramebufferAttachmentFormat::Depth_Stencil)
					return true;
			}

			return false;

		}


		static void SetTextureParameters(uint32_t textureID) {

			glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//缩小时是使用临近的四个像素加权平均
			glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		}

		
		static void SetDepthStencilTextureParameters(uint32_t textureID) {

			glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		}

		//配置帧缓冲的写入和读取目标（给一个合理默认值）的上下文配置（仅支持颜色附件）
		static void SetFramebufferDrawBuffers(uint32_t framebufferID, const FramebufferAttachmentsSpecification& attachmentsSpecification) {

			//找到第一个和最后一个颜色附件
			int firstColorAttachmentSlot = -1;		//第一个颜色附件索引
			int highestColorAttachmentSlot = -1;	//最后一个颜色附件索引

			for (int slot = 0; slot < static_cast<int>(attachmentsSpecification.AttachmentFormats.size()); slot++) {
				if (IsColorAttachment(attachmentsSpecification.AttachmentFormats[slot])) {
					if (firstColorAttachmentSlot == -1)
						firstColorAttachmentSlot = slot;

					highestColorAttachmentSlot = slot;
				}
			}

			if (highestColorAttachmentSlot == -1) {
				glNamedFramebufferDrawBuffer(framebufferID, GL_NONE);
				glNamedFramebufferReadBuffer(framebufferID, GL_NONE);
				return;
			}

			//获取附件的可写入和读取状态， 用于控制shader中fragement的out location直接对应的是槽索引（是的，location本身并不指定槽，只是偏移用于访问glNamedFramebufferDrawBuffers中传入的布局）
			std::vector<GLenum> drawBuffers;
			drawBuffers.reserve(highestColorAttachmentSlot + 1);

			for (int slot = 0; slot <= highestColorAttachmentSlot; slot++) {
				if (Utils::IsColorAttachment(attachmentsSpecification.AttachmentFormats[slot]))
					drawBuffers.emplace_back(GL_COLOR_ATTACHMENT0 + slot);
				else
					drawBuffers.emplace_back(GL_NONE);
			}

			//配置该帧缓冲的写入和读取目标上下文
			glNamedFramebufferDrawBuffers(framebufferID, static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());		//配置帧缓冲在fragement中的out location
			glNamedFramebufferReadBuffer(framebufferID, GL_COLOR_ATTACHMENT0 + firstColorAttachmentSlot);					//配置帧缓冲的默认读取（比如）
			//! ReadBuffer 指的是：当 OpenGL 要“从 framebuffer 读取颜色数据”时，应该从哪个颜色附件读。
			//! ReadBuffer：glReadPixels、glBlitFramebuffer 这类“读 framebuffer”的操作从哪里读。
			
		}

		//帧缓冲状态检测
		static void CheckFramebufferStatus(uint32_t framebufferID) {

			GLenum status = glCheckNamedFramebufferStatus(framebufferID, GL_FRAMEBUFFER);
			if (status == GL_FRAMEBUFFER_COMPLETE)
				return;

			switch (status) {
				case GL_FRAMEBUFFER_UNDEFINED:						AYIN_CORE_ERROR("FBO Error: Undefined");				break;
				case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			AYIN_CORE_ERROR("FBO Error: Incomplete Attachment");	break;
				case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	AYIN_CORE_ERROR("FBO Error: Missing Attachment");		break;
				case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:			AYIN_CORE_ERROR("FBO Error: Incomplete Draw Buffer");	break;
				case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:			AYIN_CORE_ERROR("FBO Error: Incomplete Read Buffer");	break;
				case GL_FRAMEBUFFER_UNSUPPORTED:						AYIN_CORE_ERROR("FBO Error: Unsupported Format");		break;
				case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:			AYIN_CORE_ERROR("FBO Error: Incomplete Multisample");	break;
				default:										AYIN_CORE_ERROR("FBO Error: Unknown error");
			}

			AYIN_CORE_ASSERT(false, "Framebuffer is incomplete!");

		}

	}


	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& specification)
		: Framebuffer{ specification }
	{

		Init();

	}


	OpenGLFramebuffer::~OpenGLFramebuffer() {

		uint32_t framebuffers[] = { m_FramebufferID, m_ResolveFramebufferID };
		glDeleteFramebuffers(2, framebuffers);

		glDeleteTextures(static_cast<GLsizei>(m_ColorAttachments.size()), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAndStencilAttachment);
		glDeleteTextures(1, &m_ResolveDepthAndStencilAttachment);

	}


	void OpenGLFramebuffer::Init() {

		if (m_FramebufferID != 0 || m_ResolveFramebufferID != 0) {

			uint32_t framebuffers[] = { m_FramebufferID, m_ResolveFramebufferID };
			glDeleteFramebuffers(2, framebuffers);

			glDeleteTextures(static_cast<GLsizei>(m_ColorAttachments.size()), m_ColorAttachments.data());
			glDeleteTextures(1, &m_DepthAndStencilAttachment);
			glDeleteTextures(1, &m_ResolveDepthAndStencilAttachment);

		}

		const auto& attachmentFormats = m_FramebufferSpecification.AttachmentsSpecification.AttachmentFormats;
		bool hasDepthStencilAttachment = Utils::HasDepthStencilAttachment(m_FramebufferSpecification.AttachmentsSpecification);

		//清理附件记录（主帧缓冲和解析帧缓冲）
		m_ColorAttachments.clear();
		m_ColorAttachments.resize(attachmentFormats.size(), 0);
		m_DepthAndStencilAttachment = 0;
		m_ResolveDepthAndStencilAttachment = 0;

		m_ResolveAttachmentTextures.clear();
		m_ResolveAttachmentTextures.resize(attachmentFormats.size(), nullptr);

		//创建帧缓冲
		glCreateFramebuffers(1, &m_FramebufferID);
		glCreateFramebuffers(1, &m_ResolveFramebufferID);

		//主帧缓冲和解析帧缓冲的颜色附件
		for (int slot = 0; slot < static_cast<int>(attachmentFormats.size()); slot++) {

			if (!Utils::IsColorAttachment(attachmentFormats[slot]))
				continue;

			//配置主帧缓冲颜色附件
			GLenum colorTarget = m_FramebufferSpecification.Samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
			//! 关于Switch和if，如果路径之间的差异是流程级别的，就使用switch，其余使用if，以减少样板代码
			uint32_t colorAttachment = 0;
			glCreateTextures(colorTarget, 1, &colorAttachment);

			switch (m_FramebufferSpecification.Samples) {
			
			case(1): {

				glCreateTextures(GL_TEXTURE_2D, 1, &colorAttachment);
				glTextureStorage2D(colorAttachment, 1, GL_RGBA8, m_FramebufferSpecification.Size.x, m_FramebufferSpecification.Size.y);
				Utils::SetTextureParameters(colorAttachment);

				break;
			};

			default: {

				glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &colorAttachment);
				glTextureStorage2DMultisample(
					colorAttachment,
					m_FramebufferSpecification.Samples,
					GL_RGBA8,
					m_FramebufferSpecification.Size.x,
					m_FramebufferSpecification.Size.y,
					GL_TRUE
				);

				//! 多重采样纹理是不需要设置纹理参数的

				break;
			};

			}

			m_ColorAttachments[slot] = colorAttachment;
			glNamedFramebufferTexture(m_FramebufferID, GL_COLOR_ATTACHMENT0 + slot, colorAttachment, 0);//多重采样纹理是没有minmip的，所以最后一个参数选0
			// - 0：挂 mipmap level 0，也就是原始尺寸那一层。
			// - 1：挂 mipmap level 1，也就是宽高通常减半的下一层。
			// - 对没有分配 mipmap level 1 的纹理，传 1 会导致 FBO 附件不完整。

			//配置解析缓冲颜色附件
			uint32_t resolveColorAttachment = 0;
			glCreateTextures(GL_TEXTURE_2D, 1, &resolveColorAttachment);
			glTextureStorage2D(resolveColorAttachment, 1, GL_RGBA8, m_FramebufferSpecification.Size.x, m_FramebufferSpecification.Size.y);
			Utils::SetTextureParameters(resolveColorAttachment);

			glNamedFramebufferTexture(m_ResolveFramebufferID, GL_COLOR_ATTACHMENT0 + slot, resolveColorAttachment, 0);
			m_ResolveAttachmentTextures[slot] = std::make_shared<OpenGLTexture2D>(resolveColorAttachment, m_FramebufferSpecification.Size.x, m_FramebufferSpecification.Size.y);
		
		}

		if (hasDepthStencilAttachment) {

			//深度和模板附件。这里使用 Texture 而不是 RBO，是为了后续支持采样深度信息。

			switch (m_FramebufferSpecification.Samples) {
			
			case(1): {

				glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAndStencilAttachment);
				glTextureStorage2D(m_DepthAndStencilAttachment, 1, GL_DEPTH32F_STENCIL8, m_FramebufferSpecification.Size.x, m_FramebufferSpecification.Size.y);
				Utils::SetDepthStencilTextureParameters(m_DepthAndStencilAttachment);

				break;

			};

			default: {
			
				glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &m_DepthAndStencilAttachment);
				glTextureStorage2DMultisample(
					m_DepthAndStencilAttachment,
					m_FramebufferSpecification.Samples,
					GL_DEPTH32F_STENCIL8,
					m_FramebufferSpecification.Size.x,
					m_FramebufferSpecification.Size.y,
					GL_TRUE
				);

				break;

			}
			
			}

			glNamedFramebufferTexture(m_FramebufferID, GL_DEPTH_STENCIL_ATTACHMENT, m_DepthAndStencilAttachment, 0);


			//配置解析缓冲深度模板附件
			glCreateTextures(GL_TEXTURE_2D, 1, &m_ResolveDepthAndStencilAttachment);
			glTextureStorage2D(m_ResolveDepthAndStencilAttachment, 1, GL_DEPTH32F_STENCIL8, m_FramebufferSpecification.Size.x, m_FramebufferSpecification.Size.y);
			Utils::SetDepthStencilTextureParameters(m_ResolveDepthAndStencilAttachment);
			glNamedFramebufferTexture(m_ResolveFramebufferID, GL_DEPTH_STENCIL_ATTACHMENT, m_ResolveDepthAndStencilAttachment, 0);
		
		}

		Utils::SetFramebufferDrawBuffers(m_FramebufferID, m_FramebufferSpecification.AttachmentsSpecification);
		Utils::SetFramebufferDrawBuffers(m_ResolveFramebufferID, m_FramebufferSpecification.AttachmentsSpecification);

		Utils::CheckFramebufferStatus(m_FramebufferID);
		Utils::CheckFramebufferStatus(m_ResolveFramebufferID);


		//! combind格式
		//	   格式				深度精度			模板精度			典型用途
		//	GL_DEPTH24_STENCIL8	24 - bit int		8 - bit int		通用，大多数场景
		//	GL_DEPTH32F_STENCIL8	32 - bit float		8 - bit int		高精度深度需要（大场景）
		//	两种都是 combined 格式——一块缓冲区物理上包含两种数据，GPU 按固定位偏移读写。Texture 要使用 combined 格式需要 OpenGL 4.3 + 或 ARB_stencil_texturing 扩展。

		//? RGBA8 能当深度 / 模板附件用吗？
		//!	不能。 深度和模板是 GPU 专用的存储器类型，和颜色缓冲区物理上不同。
		//	RGBA8  → 颜色附件(GPU 颜色压缩单元，按 RGBA 通道解释)
		//	D24S8  → 深度 / 模板附件(GPU 深度测试单元，按 24bit深度 + 8bit模板解释)
		//!	深度缓冲区有专门的硬件单元做深度比较（glDepthFunc）、Early - Z 剔除、Hi - Z 层级优化。拿 RGBA8 当深度附件绑，glCheckFramebufferStatus 直接返回 GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT。

		//? 分别绑定两个纹理到深度和模板附件？
		//	可以。 这也是合法做法：
		/*	// 深度纹理
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
			// 模板纹理（独立一张）
			glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX8, w, h, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, nullptr);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencilTex, 0);
		*/

	}

	void OpenGLFramebuffer::Resize(int width, int height) {

		m_FramebufferSpecification.Size.x = width;
		m_FramebufferSpecification.Size.y = height;

		Init();

	}

	void OpenGLFramebuffer::Invalidate() {}

	void OpenGLFramebuffer::Bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
	}

	void OpenGLFramebuffer::UnBind() {

		int currentFramebuffer = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFramebuffer);

		if (currentFramebuffer == m_FramebufferID)
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	Ref<Texture2D> OpenGLFramebuffer::GetColorAttachment(int index) const {

		if (index < 0 || index >= static_cast<int>(m_ResolveAttachmentTextures.size()) || m_ResolveAttachmentTextures[index] == nullptr) {
			AYIN_CORE_ASSERT(false, "Framebuffer color attachment index is not a valid color slot!");
			return nullptr;
		}

		//配置要读和写入的buffer
		glNamedFramebufferReadBuffer(m_FramebufferID, GL_COLOR_ATTACHMENT0 + index);
		glNamedFramebufferDrawBuffer(m_ResolveFramebufferID, GL_COLOR_ATTACHMENT0 + index);

		//传输
		glBlitNamedFramebuffer(
			m_FramebufferID, m_ResolveFramebufferID,
			0, 0, m_FramebufferSpecification.Size.x, m_FramebufferSpecification.Size.y,
			0, 0, m_FramebufferSpecification.Size.x, m_FramebufferSpecification.Size.y,
			GL_COLOR_BUFFER_BIT,
			GL_LINEAR
		);

		return m_ResolveAttachmentTextures[index];

	}

}
