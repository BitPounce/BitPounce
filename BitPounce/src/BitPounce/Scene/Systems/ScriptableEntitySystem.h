#pragma once

#include <BitPounce/Scene/ECSSystem.h>
#include <BitPounce/Scene/Scene.h>
#include <entt/entt.hpp>
#include "../Components.h"

namespace BitPounce
{
    class ScriptableEntitySystem : public ECSSystem
	{
	public:
		ScriptableEntitySystem() {};

		virtual void OnUpdate(Timestep& ts)
        {
            auto& m_Registry = m_Scene->GetRegistry(*this);

            m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				if (!nsc.Instance)
				{
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = Entity{ entity, m_Scene };
					nsc.Instance->OnCreate();
				}
                
				nsc.Instance->OnUpdate(ts);
			});
		}
        
	};
}