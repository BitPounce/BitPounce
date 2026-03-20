#include <bp_pch.h>
#include "ECSSystemManager.h"

namespace BitPounce
{
    void ECSSystemManager::OnRuntimeStart()
    {
		for(auto& sys : m_systems)
		{
			((ECSSystem*)sys)->OnRuntimeStart();
		}
    }

    void ECSSystemManager::OnRuntimeStop()
    {
		for(auto& sys : m_systems)
		{
			((ECSSystem*)sys)->OnRuntimeStop();
		}
    }

    void ECSSystemManager::OnEditorPropImguiDraw(Entity &entity)
    {
		for(auto& sys : m_systems)
		{
			((ECSSystem*)sys)->OnEditorPropImguiDraw(entity);
		}
    }

    void ECSSystemManager::AddComponentPopupImguiDraw(Entity &ent)
    {
		for(auto& sys : m_systems)
		{
			((ECSSystem*)sys)->AddComponentPopupImguiDraw(ent);
		}
    }

    void ECSSystemManager::OnRemoveEntity(Entity &ent)
    {
		for(auto& sys : m_systems)
		{
			((ECSSystem*)sys)->OnRemoveEntity(ent);
		}
    }

    void ECSSystemManager::Serialize(nlohmann::json& json) 
	{
		for(auto& sys : m_systems)
		{
			((ECSSystem*)sys)->Serialize(json);
		}
	}

	void ECSSystemManager::Deserialize(nlohmann::json & json)
	{
		for(auto& sys : m_systems)
		{
			((ECSSystem*)sys)->Deserialize(json);
		}
	}

    void ECSSystemManager::OnDrawEditor(Timestep &ts, EditorCamera &cam)
    {
		for(auto& sys : m_systems)
		{
			((ECSSystem*)sys)->OnDrawEditor(ts, cam);
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
