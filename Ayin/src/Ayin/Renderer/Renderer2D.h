#pragma once

#include "Ayin/Core/Core.h"
#include "Ayin/Renderer/Camera.h"
#include "Ayin/Renderer/Texture.h"

#include <glm/glm.hpp>


namespace Ayin {

	//! 一个有点突兀的设计改动，Renderer2D，Renderer2D_Data, Renderer2D::Statistics  三者的结构看起来很别扭
	//! 1、Statistics和Renderer2D_Data在面向对象的概念上应当都属于Renderer2D，可Renderer2D中并没有相应的成员变量。尽管有对应方法，这就更显的诡异了
	//x 不过Renderer::Statistics 实例是放在Renderer2D_Data中的
	//! 2、为何将Statistics定义在Renderer2D中，但是Renderer2D_Data却定义在.cpp中
	//! 
	//! 整体是两个部分Renderer2D系统和系统所需数据
	//! 我们当然可以在Renderer2D中添加对应的静态类成变，这没问题
	//! 
	//! 但是不这么做更好
	//! 对于将Renderer2D_Data和Statistics应当存储在一起我想我们没什么意见
	//!
	//! 如果我们将static Renderer2D_Data s_Data直接到Renderer2D中，.h文件需要依赖其它头文件，其它使用该头文件的头文件也将收到牵连，造成依赖地狱和编译负担
	//! 对于后期的维护也是灾难，某些时候我们只是修改了一小部分头文件，却因遭到的依赖地狱导致更大范围的重编译
	//! 底层可能是容易改动的，但是表层API可能就不怎么变动
	//!
	//! 对于用户可见性和不可见性，尽管“该做的事情一样也不会少，只不过换一个地方发生而已”
	//! 但这不代表我们就应该全放到一起，一方面是因为前面的头文件问题，另一方面开发者更关心API
	//! 用户在使用接口时需要了解到的东西（参数，返回值）应当直接放在对应文件中，以了解所接收数据的结构。所以头文件中仅展示使用API必要的东西
	//! 而对底层实现，既是为了编译器好也是为了开发者好
	//! 
	//! 这就是为什么Renderer2D_Data被放在了.cpp中，以及为什么Statistic却又被放回了，因为设计上Renderer2D_Data确实就是被开发者看的
	//! 
	//x Renderer2D设计上是一个系统，并将该静态系统视为“命名空间”
	//x 而是一个“系统”。它并不代表某个具体的“对象”（例如，你不会实例化多个渲染器），而是各项全局操作的集合。
	//x 对于系统我们更关注服务（API），而非关注组成（类中的成变组成）
	//x 
	//x 在游戏引擎开发中，我们经常跳出“面向对象”的思维，转而向“面向系统”的设计范式演进。
	//x 类通常是指你会多次实例化的对象（例如 Entity 、 Bullet 和 Particle ）。
	//x 系统（System）是一个全局单例实体（例如 Renderer 、 PhysicsWorld 和 AudioEngine ）。
	//!
	//! 原先的现在的都可以，对人来说它们没有多大心智负担(运行时也无优劣之分)，但对于编译器来说确实是有利的
	//! 
	//! 对象与数据分离-》系统与数据（黑板）
	//! 
	//! 头文件更简洁，高效编译

	AYIN_API class Renderer2D {

	public:

		static void Init();

		static void Shutdown();

		//BeginSecene是渲染的开始，在这里保证相关状态恢复到初始状态
		static void BeginScene(const Camera& camera);
		//上传数据并发起渲染
		static void EndScene();
		
		/// <summary>
		/// 发起批渲染
		/// </summary>
		static void Flush();

		static void DrawQuad(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& size, const glm::vec4& color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
		static void DrawQuad(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2{1.0f, 1.0f});


		struct Statistics {

			int DrawCalls = 0;
			int QuadCount = 0;

			inline int GetTotalQuadCount() const { return QuadCount; };
		};
		static void ResetStatistics();
		static Statistics GetStatistics();

	private:
		//当超过批处理上限时强制渲染，并恢复状态（因为该函数是提过过程中才会触发的，我们必须清理状态）
		static void FlushAndReset();

	private:


	};



}