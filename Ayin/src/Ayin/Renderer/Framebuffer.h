#pragma once

#include "Ayin/Core/Core.h"
#include "Ayin/Renderer/Texture.h"
#include <glm/glm.hpp>
#include <initializer_list>
#include <vector>

namespace Ayin {

	enum class FramebufferAttachmentFormat {

		None = 0,
		Color,
		Depth_Stencil

	};

	struct AYIN_API FramebufferAttachmentsSpecification {


		std::vector<FramebufferAttachmentFormat> AttachmentFormats = { FramebufferAttachmentFormat::Color, FramebufferAttachmentFormat::Depth_Stencil };

		FramebufferAttachmentsSpecification() = default;

		FramebufferAttachmentsSpecification(const std::initializer_list<FramebufferAttachmentFormat>& formatList) {

			// Depth_Stencil只能出现在最后，如果有但位置不正确就会将其设置为none
			// Depth_Stencil最终只有一个

			AttachmentFormats.clear();
			bool hasDepth_Stencil = false;

			//先将Depth_Stencil转换为None，为保证Depth_Stencil只出现在最后且只出现一次做准备

			for (auto format : formatList) {

				if (format == FramebufferAttachmentFormat::Depth_Stencil) {
					hasDepth_Stencil = true;
					AttachmentFormats.emplace_back(FramebufferAttachmentFormat::None);
					continue;
				}

				AttachmentFormats.emplace_back(format);

			}

			//如果过程中存在Depth_Stencil，那么就将最后一个None变为Depth_Stencil
			if (hasDepth_Stencil) {

				if (!AttachmentFormats.empty() && AttachmentFormats.back() == FramebufferAttachmentFormat::None)
					AttachmentFormats.back() = FramebufferAttachmentFormat::Depth_Stencil;
				else
					AttachmentFormats.emplace_back(FramebufferAttachmentFormat::Depth_Stencil);

			}

		}

	};

	struct AYIN_API FramebufferSpecification {

		glm::i32vec2 Size = {1280, 720};
		int Samples = 1;					// 采样数，用于MSAA

		FramebufferAttachmentsSpecification AttachmentsSpecification;

		bool SwapChainTarget = false;
	};



	class AYIN_API Framebuffer {


	public:

		virtual ~Framebuffer() = default;

		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		virtual operator uint32_t() = 0;

		virtual void Resize(int width, int height) = 0;

		virtual Ref<Texture2D> GetColorAttachment(int index = 0) const = 0;

		const FramebufferSpecification& GetSpecification() { return m_FramebufferSpecification; };

	protected:

		inline Framebuffer(const FramebufferSpecification& specification)
			:m_FramebufferSpecification{ specification } 
		{};

	protected:

		FramebufferSpecification m_FramebufferSpecification;

	public:

		static Ref<Framebuffer> Create(const FramebufferSpecification& specification);


	};

}
