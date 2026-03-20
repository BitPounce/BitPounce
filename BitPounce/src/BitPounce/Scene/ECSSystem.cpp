#include <bp_pch.h>
#include "ECSSystem.h"
#include "Entity.h"

namespace BitPounce
{

	void BitPounce::ECSSystem::INIT_INT(Scene* scene)
	{
		m_Scene = scene;
	}

	template<typename T>
	void ECSSystem::OnComponentRemoved(Entity entity, T& component)
	{
	}
}
