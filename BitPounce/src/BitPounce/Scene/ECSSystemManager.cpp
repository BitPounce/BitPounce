#include <bp_pch.h>
#include "ECSSystemManager.h"

namespace BitPounce
{
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
