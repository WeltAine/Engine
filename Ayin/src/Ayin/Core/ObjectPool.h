#pragma once

#include "Ayin/Core/Core.h"
#include <vector>
#include <concepts>


template <typename T>
	requires std::default_initializable<T>
class ObjectPool {

public:

	ObjectPool(int chunkSize = 32) 
		:m_ChunkSize{chunkSize} {

	}

T* Allocate() {

		if (!m_FreeList) {

			AllocateChunks();

		}

		Node* node = m_FreeList;
		m_FreeList = m_FreeList->Next;

		T* obj = reinterpret_cast<T*>(node);
		m_Allocated.push_back(obj);
		return obj;
	}

	void Deallocate(T* obj) {

		obj->~T();

		m_Allocated.erase(
			std::remove(m_Allocated.begin(), m_Allocated.end(), obj),
			m_Allocated.end()
		);

		Node* node = reinterpret_cast<Node*>(obj);
		node->Next = m_FreeList;
		m_FreeList = node;

	}

	~ObjectPool() {

		for (T* obj : m_Allocated) {

			obj->~T();

		}

		m_Allocated.clear();

		for (void* chunk : m_Chunks) {

			::operator delete(chunk);

		}

	}

protected:

	union Node {

		alignas(T) char storage[sizeof(T)];	// 存储池对象数据（保证对象地址对齐）
		Node* Next;							// 无用时用来存储池中下一个可用内存的地址(不能时void*？？？)

	};


	void AllocateChunks() {

		//开辟空间但是不初始化
		Node* chunks = static_cast<Node*>(::operator new(sizeof(Node)* m_ChunkSize));

		for (int index = 0; index < m_ChunkSize - 1; index++) {

			chunks[index].Next = &chunks[index + 1];

		}

		chunks[m_ChunkSize - 1].Next = nullptr;

		m_FreeList = chunks;

		m_Chunks.push_back(chunks);
	}


protected:

	Node* m_FreeList = nullptr;
	std::vector<void*> m_Chunks;
	std::vector<T*> m_Allocated;
	int m_ChunkSize = 0;

};
