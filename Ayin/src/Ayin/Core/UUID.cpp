#include <AyinPch.h>

#include "Ayin/Core/UUID.h"


namespace Ayin {

	std::unordered_set<uint64_t> UUIDGenerator::s_GlobalUniqueIDSet{ 0 };				// 正在使用的UUID


	std::mt19937_64 UUIDGenerator::s_RandomEngine{ [] {
#ifdef AYIN_DEBUG
				return std::mt19937_64(42);				// 固定种子，调试时可复现
#else
				static std::random_device rd;
				return std::mt19937_64(rd());
#endif
				}() };									// 随机数生成器算法

	std::uniform_int_distribution<uint64_t> UUIDGenerator::s_UniformDistribution;	//!均匀分布在[0, 2 ^ 64 - 1] 区间。对 UUID 这正好是需要的——64 位全随机，碰撞概率可忽略。





	UUID UUIDGenerator::GenerateUniqueID() {
	
		UUID globalUniqueID = 0;

		while (!IsUUIDUnused(globalUniqueID)) {
			globalUniqueID = s_UniformDistribution(s_RandomEngine);
		}

		s_GlobalUniqueIDSet.insert(globalUniqueID);

		return globalUniqueID;
	
	};

	void UUIDGenerator::NullifyUUID(UUID globalUniqueID) {
	
		s_GlobalUniqueIDSet.erase(globalUniqueID);

	};

	bool UUIDGenerator::IsUUIDUnused(UUID globalUniqueID) {

		return !s_GlobalUniqueIDSet.contains(globalUniqueID);

	};

	bool UUIDGenerator::IsUUIDAlive(UUID globalUniqueID) {

		return s_GlobalUniqueIDSet.contains(globalUniqueID) && globalUniqueID != 0;

	}








};
