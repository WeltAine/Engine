#include <AyinPch.h>

#include "Ayin/Scene/Entity.h"

namespace Ayin {


	Entity::Entity(Scene* scene) 
		:m_Scene{scene}
	{
	
		if (m_Scene != nullptr)
			m_EntityHandle = m_Scene->m_Registry.create();

	};


}