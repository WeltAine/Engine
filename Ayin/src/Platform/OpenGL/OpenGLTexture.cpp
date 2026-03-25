#include <AyinPch.h>

#include "OpenGLTexture.h"
#include "stb_image.h"

#include "glad/glad.h"



namespace Ayin {

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path) 
		: m_Path(path)
	{

		stbi_set_flip_vertically_on_load(1);//设置stb_image的读取顺序

		//加载数据				  //路径		//图片宽  //图片高	 //像素通道数	//输出通道数量
		stbi_uc* data = stbi_load(path.c_str(), &m_Width, &m_Height, &m_Components, 0);//stbi_uc是unsigned char
		AYIN_CORE_ASSERT(data, "Failed to load image");
		//! OpenGL对于图片数据的期望是从底到顶，而stb_image默认是从顶到底读取


		GLenum internalformat = 0, dataformat = 0;

		switch (m_Components) {

			case(3): {
				internalformat = GL_RGB8;
				dataformat = GL_RGB;
				break;
			};
			case(4): {
				internalformat = GL_RGBA8;
				dataformat = GL_RGBA;
				break;
			};
			default: {
				AYIN_CORE_ASSERT(internalformat & dataformat, "Format not supported");
			}

		}

		//创建对象，开辟空间
		//! 比旧API好用多了，旧API是靠当前激活槽位关注要设置的对象的，新API直接关注对象
		glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);
		glTextureStorage2D(m_TextureID, 1, internalformat, m_Width, m_Height);
		//x 预留一块内存空间，确保每个像素的每个通道精确占用 8 位。
		//x 这就是 OpenGL 4.2 引入的“不可变存储（Immutable Storage）”模型
		//x 其目的是避免在后续更改纹理时，驱动程序因重新分配内存而产生开销。"

		//参数设置
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//缩小时是使用临近的四个像素加权平均
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//上传数据													 //每像素多少通道（格式）	//每通道的数据类型（类型）
		glTextureSubImage2D(m_TextureID, 0, 0, 0, m_Width, m_Height, dataformat,				GL_UNSIGNED_BYTE, data);//x 一旦“形状”（glTextureStorage的固定式分配每一层的空间大小，但不只是考虑大小而是像4*4放不进你的3*3保险箱那样，就像）无法兼容，则报错
		//? 为什么GL_RBG8不行呢，它明明看上去更准确？
		//! glTextureStorage2D中的internalFormat和glTextureSubImage2D中的Format作用是不一样的
		//! glTextureStorage2D是描述GPU中的存储格式，而glTextureSubImage2D则是通过Format（不带位深度） + Type共同描述CPU中的存储格式
		//! 其实没什么好纠结的，OpenGL的API设计确实不怎么好
		//x 在图形学发展的早期（20 世纪 90 年代），CPU 内存与 GPU 内存的架构往往大相径庭。
		//x CPU 通常以简单的线性数组形式存储数据，而 GPU 则可能采用“平铺（tiling）”或“重排（swizzling）”技术来优化内存布局（你可能在计算机组成原理上看到过那个网状一样多维空间般的内存布局，以保证n步内访问到任何数据），从而提升硬件读取相邻像素的效率。

		//删除资源
		stbi_image_free(data);

	}

	OpenGLTexture2D::~OpenGLTexture2D() {

		glDeleteTextures(1, &m_TextureID);

	}




	void OpenGLTexture2D::Bind(int slot) const
	{
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, m_TextureID);
		
		//glBindTextures();
		//glBindImageTexture();

		glBindTextureUnit(slot, m_TextureID);

	}

	void OpenGLTexture2D::UnBind() const
	{

		int maxTextureUnits;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
		int currentActiveUint;
		glGetIntegerv(GL_ACTIVE_TEXTURE, &currentActiveUint);

		for(int uint = 0; uint < maxTextureUnits; uint++) {

			glActiveTexture(uint);//! 旧版API在使用是必须先激活槽，才能glGetIntegerv查询上头的纹理单元，没有可以替代的新版方案

			int uintTextureID = 0;//单元中的纹理对象
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &uintTextureID);

			if (uintTextureID != m_TextureID)
				continue;

			glBindTexture(GL_TEXTURE_2D, 0);//一个纹理可能被设定到了多个槽位中

		};

		glActiveTexture(currentActiveUint);

	}


}

