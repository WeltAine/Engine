#include <AyinPch.h>

#include "Ayin/Math/Math.h"

#include <glm/gtx/orthonormalize.hpp> // 正确的头文件


namespace Ayin::Math {

	std::tuple<glm::vec3, glm::vec3, glm::vec3> DecomposeTransform(const glm::mat4& transform) {
	
		using T = float;

		glm::mat4 localMatrix{ transform };//视为全局变换

		if (glm::epsilonEqual<T>(localMatrix[3][3], static_cast<float>(0), glm::epsilon<T>()))
			return { glm::vec3{ 0.0f }, glm::vec3{ 0.0f }, glm::vec3{0.0f} };

		//处理数乘
		if (glm::epsilonNotEqual<T>(localMatrix[3][3], static_cast<float>(1), glm::epsilon<T>())) {
			localMatrix = localMatrix / localMatrix[3][3];
		}

		//处理平移
		glm::vec3 translate = localMatrix[3];

		//处理缩放
		glm::vec3 scale{ 1.0f };
		scale.x = glm::length(localMatrix[0]);
		scale.y = glm::length(localMatrix[1]);
		scale.z = glm::length(localMatrix[2]);

		glm::mat3 rotationMatrix = glm::orthonormalize(glm::mat3{ localMatrix });

		//修复因缩放导致的坐标系切换
		if (glm::dot(glm::cross(rotationMatrix[0], rotationMatrix[1]), rotationMatrix[2]) < 0) {
			
			//! 只有两种系，左手和右手，而两者之间一定可行的切换就是乘以-1
			scale *= glm::vec3{-1.0f};
			rotationMatrix *= -1.0f;
		
		}

		//旋转（返回弧度值）
		glm::vec3 rotation{ 0.0f };
		rotation.y = std::asin(rotationMatrix[2][0]);
		if (std::cos(rotation.y) != 0) {
			//非0时下面的四个元素才时非0的
			rotation.x = std::atan2(-rotationMatrix[2][1], rotationMatrix[2][2]);
			rotation.z = std::atan2(-rotationMatrix[1][0], rotationMatrix[0][0]);
		
		}else {
			//为0时需要使用等价表达
			rotation.x = std::atan2(rotationMatrix[0][1], rotationMatrix[1][1]);
			rotation.z = 0.0f;

		}
		
		return {translate, rotation, scale};

	}

}
