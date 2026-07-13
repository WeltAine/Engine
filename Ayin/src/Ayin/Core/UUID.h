#pragma once

#include "Ayin/Core/Core.h"
#include <random>
#include <unordered_set>
#include <cstdint>

namespace Ayin {

	class UUIDGenerator {

	public:

		//Static void Init();

		static UUID GenerateUniqueID();

		static void	NullifyUUID(UUID globalUniqueID);

		static bool IsUUIDUnused(UUID globalUniqueID);

		static bool IsUUIDAlive(UUID globalUniqueID);
		

	private:

		static std::unordered_set<uint64_t> s_GlobalUniqueIDSet;				// 正在使用的UUID
		
		static std::mt19937_64 s_RandomEngine;									// 随机数生成器算法
		static std::uniform_int_distribution<uint64_t> s_UniformDistribution;	//!均匀分布在[0, 2 ^ 64 - 1] 区间。对 UUID 这正好是需要的——64 位全随机，碰撞概率可忽略。
		
	};


};
