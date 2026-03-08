#include <bp_pch.h>
#include "ECSSystemManager.h"

namespace BitPounce
{
    void ECSSystemManager::OnEditorPropImguiDraw(Entity &entity)
    {
		for(auto& sys : m_systems)
		{
			((ECSSystem*)sys)->OnEditorPropImguiDraw(entity);
		}
    }

    void ECSSystemManager::AddSys_in(System *sys)
	{

		if(!dynamic_cast<ECSSystem*>(sys))
		{
			BP_CORE_ASSERT(0, "SYS IS NOT BASED IN ECSSystem");
			return;
		}
		((ECSSystem*)sys)->INIT_INT(m_Scene);

		SystemManager::AddSys_in(sys);

	}
}
