#pragma once

#include "Ayin/Renderer/Texture.h"

#include <Glad/glad.h>


namespace Ayin {

	class AYIN_API OpenGLTexture2D : public Texture2D {

	public:
		OpenGLTexture2D(const std::string& path);
		OpenGLTexture2D(int width, int height, void* data = nullptr);

		virtual ~OpenGLTexture2D() override;

		// 通过 Texture2D 继承
		virtual void SetData(int width, int height, void* datar) override;

		virtual inline int GetWidth() const override { return m_Width; };
		virtual inline int GetHeight() const override { return m_Height; };
		virtual inline int GetComponents() const override { return m_Components; };

		virtual void Bind(int slot) const override;

		virtual void UnBind() const override;


	private:

		int m_Width, m_Height;	//宽高
		GLenum m_InternalFormat, m_DataFormat;
		int m_Components;		//通道数量
		uint32_t m_TextureID;

		std::string m_Path; //记录路径，可以实现热更新支持（虽然这次我们没有实现，谁知道什么时候呢）

	};


}