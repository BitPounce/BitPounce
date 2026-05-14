#include <bp_pch.h>
#include "2DPhysicsSystem.h"

namespace BitPounce
{
		template<>
		void ECSSystem::OnComponentAdded<DistanceJoint2D>(Entity entity, DistanceJoint2D& component)
		{
			Physics2DSystem* sys = dynamic_cast<Physics2DSystem*>(this);
			if(!sys) { return; }
			if (sys->m_IsInRuntime && b2World_IsValid(sys->m_PhysicsWorld))
				sys->CreateDistanceJoint(entity);
		}

		template<>
		void ECSSystem::OnComponentRemoved<DistanceJoint2D>(Entity entity, DistanceJoint2D& component)
		{
			Physics2DSystem* sys = dynamic_cast<Physics2DSystem*>(this);
			if(!sys) { return; }
			if (sys->m_IsInRuntime && b2World_IsValid(sys->m_PhysicsWorld))
				sys->DestroyDistanceJoint(component);
		}
}