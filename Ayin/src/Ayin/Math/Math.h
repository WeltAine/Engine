#pragma once

#include <Ayin.h>
#include <glm/glm.hpp>

namespace Ayin::Math {

	/// <summary>
	/// 分解矩阵（语义上只适配于经历平移，缩放，旋转和数乘的矩阵，再主空间理解下分解）
	/// 换句话说，它只会认为矩阵经历了上述四种操作，并以此为基础分解，哪怕你的矩阵经历了透视变换
	/// </summary>
	/// <param name="transform">位置，旋转（弧度制），缩放</param>
	/// <returns></returns>
	std::tuple<glm::vec3, glm::vec3, glm::vec3> DecomposeTransform(const glm::mat4& transform);

}
