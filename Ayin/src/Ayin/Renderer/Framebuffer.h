#pragma once

#include "Ayin/Core/Core.h"
#include "Ayin/Renderer/Texture.h"

namespace Ayin {


	struct AYIN_API FramebufferSpecification {

		int Width, Height;
		int Samples = 1;		//? 这个参数是什么意思

		//Todo 缓冲格式（缓冲类型）
		// FramebufferFormat Format;

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
