#include <bp_pch.h>
#include "SystemManager.h"

namespace BitPounce
{
    SystemManager::~SystemManager()
    {
		for (auto& sys : m_systems)
		{
			delete sys;
		}
		m_systems.clear();
		
    }

    void SystemManager::OnUpdate(Timestep& ts)
	{
		for (auto& sys : m_systems)
		{
			sys->OnUpdate(ts);
		}
	}

    void SystemManager::OnDraw(Timestep &ts)
    {
		for (auto& sys : m_systems)
		{
			sys->OnDraw(ts);
		}
    }

    void SystemManager::OnEvent(Event& event)
	{
		for (auto& sys : m_systems)
		{
			sys->OnEvent(event);
		}
	}

	void SystemManager::OnImGuiDraw()
	{
		for (auto& sys : m_systems)
		{
			sys->OnImGuiDraw();
		}
	}

    void SystemManager::Start()
    {
		for (auto& sys : m_systems)
		{
			sys->Start();
		}
    }

    void SystemManager::Stop()
    {
		for (auto& sys : m_systems)
		{
			sys->Stop();
		}
    }

    void SystemManager::StopSystem(System* sys)
	{
		for (auto* s : m_systems)
		{
			if (s == sys)
			{
				s->Stop();
				return;
			}
		}
	}

    void SystemManager::StartSystem(System* sys)
    {
		for (auto* s : m_systems)
		{
			if (s == sys)
			{
				s->Start();
				return;
			}
		}
    }

    void SystemManager::AddSys_in(System* sys)
	{
		m_systems.push_back(sys);
	}

}