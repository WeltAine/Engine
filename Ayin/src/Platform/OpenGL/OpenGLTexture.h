#pragma once

#include "Ayin/Renderer/Texture.h"

#include <Glad/glad.h>


namespace Ayin {

	class AYIN_API OpenGLTexture2D : public Texture2D {

	public:
		OpenGLTexture2D(const std::string& path);
		OpenGLTexture2D(int width, int height, void* data = nullptr);

		//非常规构造（我们的纹理实现是相当简单的，其它模块可能需要更复杂的纹理，这样的设计可以保证其它方式创建的模可以以Texture2D的方式存在，同时享受Ref<资源管理机制的好处>）,目前仅支持RGBA8
		
		/// <summary>
		/// 适配器模式，可以将各种GPU纹理变为Ayin::Texture
		/// 主要针对非常规构造（我们的纹理实现是相当简单的，其它模块可能需要更复杂的纹理，这样的设计可以保证其它方式创建的模可以以Texture2D的方式存在，同时享受Ref<资源管理机制的好处>）
		/// 目前仅支持RGBA8
		/// </summary>
		/// <param name="textureID"></param>
		/// <param name="width"></param>
		/// <param name="height"></param>
		OpenGLTexture2D(uint32_t textureID, int width, int height);

		virtual ~OpenGLTexture2D() override;

		// 通过 Texture2D 继承
		virtual void SetData(int width, int height, void* datar) override;

		virtual inline int GetWidth() const override { return m_Width; };
		virtual inline int GetHeight() const override { return m_Height; };
		virtual inline int GetComponents() const override { return m_Components; };

		virtual void Bind(int slot) const override;

		virtual void UnBind() const override;


		virtual operator uint32_t() const { return m_TextureID; };
		virtual bool operator == (const Texture& other) const override;

	private:

		int m_Width, m_Height;	//宽高
		GLenum m_InternalFormat, m_DataFormat;
		int m_Components;		//通道数量
		uint32_t m_TextureID;

		std::string m_Path; //记录路径，可以实现热更新支持（虽然这次我们没有实现，谁知道什么时候呢）

	};


}